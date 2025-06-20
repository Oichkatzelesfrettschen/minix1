/* This file deals with creating processes (via FORK) and deleting them (via
 * EXIT/WAIT).  When a process forks, a new slot in the 'mproc' table is
 * allocated for it, and a copy of the parent's core image is made for the
 * child.  Then the kernel and file system are informed.  A process is removed
 * from the 'mproc' table when two events have occurred: (1) it has exited or
 * been killed by a signal, and (2) the parent has done a WAIT.  If the process
 * exits first, it continues to occupy a slot until the parent does a WAIT.
 *
 * The entry points into this file are:
 *   do_fork:	perform the FORK system call
 *   do_mm_exit:	perform the EXIT system call (by calling mm_exit())
 *   mm_exit:	actually do the exiting
 *   do_wait:	perform the WAIT system call
 */

#include "../h/callnr.hpp"
#include "../h/const.hpp"
#include "../h/error.hpp"
#include "../h/type.hpp" // Defines phys_clicks, vir_clicks, CLICK_SHIFT etc.
#include "alloc.hpp"
#include "const.hpp"
#include "glo.hpp"
#include "mproc.hpp"
#include "param.hpp"
#include "token.hpp"
#include <cstddef> // For std::size_t
#include <cstdint> // For uint64_t

#define LAST_FEW 2 /* last few slots reserved for superuser */

PRIVATE next_pid = INIT_PROC_NR + 1; /* next pid to be assigned */

/* Some C compilers require static declarations to precede their first use. */

/*===========================================================================*
 *				do_fork					     *
 *===========================================================================*/
PUBLIC int do_fork() {
    /* The process pointed to by 'mp' has forked.  Create a child process. */

    register struct mproc *rmp; /* pointer to parent */
    register struct mproc *rmc; /* pointer to child */
    int i, child_nr, t;
    char *sptr, *dptr;
    uint64_t prog_bytes;              // Was long, should be phys_bytes equivalent
    uint64_t prog_clicks, child_base; // phys_clicks -> uint64_t
    uint64_t parent_abs, child_abs;   // Was long, should be phys_bytes equivalent
    // extern phys_clicks alloc_mem(); // alloc_mem now returns uint64_t

    /* If tables might fill up during FORK, don't even start since recovery half
     * way through is such a nuisance.
     */

    rmp = mp;
    if (procs_in_use == NR_PROCS)
        return (ErrorCode::EAGAIN);
    if (procs_in_use >= NR_PROCS - LAST_FEW && rmp->mp_effuid != 0)
        return (ErrorCode::EAGAIN);

    /* Determine how much memory to allocate. */
    // mp_seg[X].mem_len are vir_clicks (std::size_t). Sum is std::size_t.
    // Cast to phys_clicks (uint64_t) is appropriate for physical allocation size.
    prog_clicks = static_cast<uint64_t>(rmp->mp_seg[T].mem_len + rmp->mp_seg[D].mem_len +
                                        rmp->mp_seg[S].mem_len);
    prog_bytes = prog_clicks << CLICK_SHIFT;             // prog_clicks is uint64_t
    if ((child_base = alloc_mem(prog_clicks)) == NO_MEM) // alloc_mem takes uint64_t
        return (ErrorCode::EAGAIN);

    /* Create a copy of the parent's core image for the child. */
    child_abs = child_base << CLICK_SHIFT;               // child_base is uint64_t
    parent_abs = rmp->mp_seg[T].mem_phys << CLICK_SHIFT; // mem_phys is uint64_t
    // New mem_copy signature: (int, int, uintptr_t, int, int, uintptr_t, std::size_t)
    // parent_abs, child_abs, prog_bytes are uint64_t.
    // Assuming physical addresses fit in uintptr_t and counts in std::size_t.
    i = mem_copy(ABS, 0, static_cast<uintptr_t>(parent_abs), ABS, 0,
                 static_cast<uintptr_t>(child_abs), static_cast<std::size_t>(prog_bytes));
    if (i < 0)
        panic("do_fork can't copy", i);

    /* Find a slot in 'mproc' for the child process.  A slot must exist. */
    for (rmc = &mproc[0]; rmc < &mproc[NR_PROCS]; rmc++)
        if ((rmc->mp_flags & IN_USE) == 0)
            break;

    /* Set up the child and its memory map; copy its 'mproc' slot from parent. */
    child_nr = rmc - mproc; /* slot number of the child */
    procs_in_use++;
    sptr = (char *)rmp;       /* pointer to parent's 'mproc' slot */
    dptr = (char *)rmc;       /* pointer to child's 'mproc' slot */
    i = sizeof(struct mproc); /* number of bytes in a proc slot. */
    while (i--)
        *dptr++ = *sptr++; /* copy from parent slot to child's */

    rmc->mp_parent = who;                 /* record child's parent */
    rmc->mp_seg[T].mem_phys = child_base; // child_base is uint64_t
    // mem_len is std::size_t (vir_clicks), child_base is uint64_t (phys_clicks).
    // mem_phys is uint64_t (phys_clicks)
    rmc->mp_seg[D].mem_phys = child_base + static_cast<uint64_t>(rmc->mp_seg[T].mem_len);
    // All mp_seg[X].mem_phys are phys_clicks (uint64_t)
    rmc->mp_seg[S].mem_phys =
        rmc->mp_seg[D].mem_phys + (rmp->mp_seg[S].mem_phys - rmp->mp_seg[D].mem_phys);
    rmc->mp_exitstatus = 0;
    rmc->mp_sigstatus = 0;
    rmc->mp_token = generate_token();

    /* Find a free pid for the child and put it in the table. */
    do {
        t = 0; /* 't' = 0 means pid still free */
        next_pid = (next_pid < 30000 ? next_pid + 1 : INIT_PROC_NR + 1);
        for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++)
            if (rmp->mp_pid == next_pid) {
                t = 1;
                break;
            }
        rmc->mp_pid = next_pid; /* assign pid to child */
    } while (t);

    /* Tell kernel and file system about the (now successful) FORK. */
    sys_fork(who, child_nr, rmc->mp_pid, rmc->mp_token);
    tell_fs(FORK, who, child_nr, 0);

    /* Report child's memory map to kernel. */
    sys_newmap(child_nr, rmc->mp_seg);

    /* Reply to child to wake it up. */
    reply(child_nr, 0, 0, NIL_PTR);
    return (next_pid); /* child's pid */
}

/*===========================================================================*
 *				do_mm_exit				     *
 *===========================================================================*/
PUBLIC int do_mm_exit() {
    /* Perform the exit(status) system call. The real work is done by mm_exit(),
     * which is also called when a process is killed by a signal.
     */

    mm_exit(mp, status);
    dont_reply = TRUE; /* don't reply to newly terminated process */
    return (OK);       /* pro forma return code */
}

/*===========================================================================*
 *				mm_exit					     *
 *===========================================================================*/
PUBLIC mm_exit(rmp, exit_status)
register struct mproc *rmp; /* pointer to the process to be terminated */
int exit_status;            /* the process' exit status (for parent) */
{
    /* A process is done.  If parent is waiting for it, clean it up, else hang. */

    /* How to terminate a process is determined by whether or not the
     * parent process has already done a WAIT.  Test to see if it has.
     */
    rmp->mp_exitstatus = (char)exit_status; /* store status in 'mproc' */

    if (mproc[rmp->mp_parent].mp_flags & WAITING)
        cleanup(rmp); /* release parent and tell everybody */
    else
        rmp->mp_flags |= HANGING; /* Parent not waiting.  Suspend proc */

    /* If the exited process has a timer pending, kill it. */
    if (rmp->mp_flags & ALARM_ON)
        set_alarm(rmp - mproc, (unsigned)0);

    /* Tell the kernel and FS that the process is no longer runnable. */
    sys_xit(rmp->mp_parent, rmp - mproc);
    tell_fs(EXIT, rmp - mproc, 0, 0); /* file system can free the proc slot */
}

/*===========================================================================*
 *				do_wait					     *
 *===========================================================================*/
PUBLIC int do_wait() {
    /* A process wants to wait for a child to terminate. If one is already waiting,
     * go clean it up and let this WAIT call terminate.  Otherwise, really wait.
     */

    register struct mproc *rp;
    register int children;

    /* A process calling WAIT never gets a reply in the usual way via the
     * reply() in the main loop.  If a child has already exited, the routine
     * cleanup() sends the reply to awaken the caller.
     */

    /* Is there a child waiting to be collected? */
    children = 0;
    for (rp = &mproc[0]; rp < &mproc[NR_PROCS]; rp++) {
        if ((rp->mp_flags & IN_USE) && rp->mp_parent == who) {
            children++;
            if (rp->mp_flags & HANGING) {
                cleanup(rp); /* a child has already exited */
                dont_reply = TRUE;
                return (OK);
            }
        }
    }

    /* No child has exited.  Wait for one, unless none exists. */
    if (children > 0) { /* does this process have any children? */
        mp->mp_flags |= WAITING;
        dont_reply = TRUE;
        return (OK); /* yes - wait for one to exit */
    } else
        return (ErrorCode::ECHILD); /* no - parent has no children */
}

/*===========================================================================*
 *				cleanup					     *
 *===========================================================================*/
PRIVATE cleanup(child)
register struct mproc *child; /* tells which process is exiting */
{
    /* Clean up the remains of a process.  This routine is only called if two
     * conditions are satisfied:
     *     1. The process has done an EXIT or has been killed by a signal.
     *     2. The process' parent has done a WAIT.
     *
     * It releases the memory, if that has not been done yet.  Whether it has or
     * has not been done depends on the order of the EXIT and WAIT calls.
     */

    register struct mproc *parent, *rp;
    int init_waiting, child_nr;
    unsigned int r; // For status, not a target type for this change
    uint64_t s;     // phys_clicks -> uint64_t

    child_nr = child - mproc;
    parent = &mproc[child->mp_parent];

    /* Wakeup the parent. */
    r = child->mp_sigstatus & 0377;
    r = r | (child->mp_exitstatus << 8);
    reply(child->mp_parent, child->mp_pid, r, NIL_PTR);

    /* Release the memory occupied by the child. */
    // child->mp_seg[X].mem_vir and mem_len are vir_clicks (std::size_t)
    // s is uint64_t (phys_clicks)
    s = static_cast<uint64_t>(child->mp_seg[S].mem_vir + child->mp_seg[S].mem_len);
    if (child->mp_flags & SEPARATE)
        s += static_cast<uint64_t>(child->mp_seg[T].mem_len);
    // child->mp_seg[T].mem_phys is phys_clicks (uint64_t)
    // free_mem takes (uint64_t, uint64_t)
    free_mem(child->mp_seg[T].mem_phys, s); /* free the memory */

    /* Update flags. */
    child->mp_flags &= ~HANGING;  /* turn off HANGING bit */
    child->mp_flags &= ~PAUSED;   /* turn off PAUSED bit */
    parent->mp_flags &= ~WAITING; /* parent is no longer waiting */
    child->mp_flags &= ~IN_USE;   /* release the table slot */
    procs_in_use--;

    /* If exiting process has children, disinherit them.  INIT is new parent. */
    init_waiting = (mproc[INIT_PROC_NR].mp_flags & WAITING ? 1 : 0);
    for (rp = &mproc[0]; rp < &mproc[NR_PROCS]; rp++) {
        if (rp->mp_parent == child_nr) {
            /* 'rp' points to a child to be disinherited. */
            rp->mp_parent = INIT_PROC_NR; /* init takes over */
            if (init_waiting && (rp->mp_flags & HANGING)) {
                /* Init was waiting. */
                cleanup(rp); /* recursive call */
                init_waiting = 0;
            }
        }
    }
}
