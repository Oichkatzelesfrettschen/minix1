#pragma once
// Modernized for C++17

/* Here is the declaration of the process table.  Three assembly code routines
 * reference fields in it.  They are restart(), save(), and csv().  When
 * changing 'proc', be sure to change the field offsets built into the code.
 * It contains the process' registers, memory map, accounting, and message
 * send/receive information.
 */
#include "../include/defs.h"
#include "type.hpp"

EXTERN struct proc {
    u64_t p_reg[NR_REGS];          /* process' registers */
    u64_t p_sp;                    /* stack pointer */
    struct pc_psw p_pcpsw;         /* pc and psw as pushed by interrupt */
    int p_flags;                   /* P_SLOT_FREE, SENDING, RECEIVING, etc. */
    struct mem_map p_map[NR_SEGS]; /* memory map */
    u64_t p_splimit;               /* lowest legal stack value */
    int p_pid;                     /* process id passed in from MM */

    real_time user_time;   /* user time in ticks */
    real_time sys_time;    /* sys time in ticks */
    real_time child_utime; /* cumulative user time of children */
    real_time child_stime; /* cumulative sys time of children */
    real_time p_alarm;     /* time of next alarm in ticks, or 0 */

    struct proc *p_callerq;  /* head of list of procs wishing to send */
    struct proc *p_sendlink; /* link to next proc wishing to send */
    message *p_messbuf;      /* pointer to message buffer */
    int p_getfrom;           /* from whom does process want to receive? */

    struct proc *p_nextready; /* pointer to next ready process */
    int p_pending;            /* bit map for pending signals 1-16 */
    u64_t cr3;                /* page table base */
    int p_priority;           /* scheduling priority */
    int p_cpu;                /* CPU affinity */
} proc[NR_TASKS + NR_PROCS];

/* Bits for p_flags in proc[].  A process is runnable iff p_flags == 0 */
#define P_SLOT_FREE 001 /* set when slot is not in use */
#define NO_MAP 002      /* keeps unmapped forked child from running */
#define SENDING 004     /* set when process blocked trying to send */
#define RECEIVING 010   /* set when process blocked trying to recv */

#define proc_addr(n) &proc[NR_TASKS + n]
#define NIL_PROC (struct proc *)0

EXTERN struct proc *proc_ptr;                        /* &proc[cur_proc] */
EXTERN struct proc *bill_ptr;                        /* ptr to process to bill for clock ticks */
EXTERN struct proc *rdy_head[NR_CPUS][SCHED_QUEUES]; /* per-CPU ready list heads */
EXTERN struct proc *rdy_tail[NR_CPUS][SCHED_QUEUES]; /* per-CPU ready list tails */

EXTERN unsigned busy_map;                /* bit map of busy tasks */
EXTERN message *task_mess[NR_TASKS + 1]; /* ptrs to messages for busy tasks */
