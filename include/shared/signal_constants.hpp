#pragma once

// Common signal number definitions and helper types shared
// between the kernel and user space. These constants
// mirror the historical MINIX signal assignments.

#define NR_SIGS 16 /* number of signals used */
#define NSIG 16    /* number of signals used */

#define SIGHUP 1   /* hangup */
#define SIGINT 2   /* interrupt (DEL) */
#define SIGQUIT 3  /* quit (ASCII FS) */
#define SIGILL 4   /* illegal instruction (not reset when caught)*/
#define SIGTRAP 5  /* trace trap (not reset when caught) */
#define SIGIOT 6   /* IOT instruction */
#define SIGEMT 7   /* EMT instruction */
#define SIGFPE 8   /* floating point exception */
#define SIGKILL 9  /* kill (cannot be caught or ignored) */
#define SIGBUS 10  /* bus error */
#define SIGSEGV 11 /* segmentation violation */
#define SIGSYS 12  /* bad argument to system call */
#define SIGPIPE 13 /* write on a pipe with no one to read it */
#define SIGALRM 14 /* alarm clock */
#define SIGTERM 15 /* software termination signal from kill */

#define STACK_FAULT 16 /* used by kernel to signal stack fault */

typedef void (*sighandler_t)(int);

// The signal() function installs a handler for the given signal number
// and returns the previous handler.
sighandler_t signal(int signum, sighandler_t handler);

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
