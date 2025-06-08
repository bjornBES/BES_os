#pragma once

#include "defaultInclude.h"
#include "types.h"

#define NSIG 17
#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR		30
#define SIGSYS		31
#define	SIGUNUSED	31

/* These should not be considered constants from userland.  */
#define SIGRTMIN	32
#ifndef SIGRTMAX
#define SIGRTMAX	NSIG
#endif

#if !defined MINSIGSTKSZ || !defined SIGSTKSZ
#define MINSIGSTKSZ	2048
#define SIGSTKSZ	8192
#endif

#ifndef __ASSEMBLY__
/* not actually used, but required for linux/syscalls.h */
typedef unsigned long old_sigset_t;

#include "signal-defs.h"

#ifdef SA_RESTORER
#define __ARCH_HAS_SA_RESTORER
#endif

#ifndef __KERNEL__
struct sigaction {
	sighandler_t sa_handler;
	unsigned long sa_flags;
    sigset_t sa_mask;
};
#endif

typedef struct sigaltstack {
	void *ss_sp;
	int ss_flags;
} stack_t;

#endif /* __ASSEMBLY__ */

sighandler_t signal(int signum, sighandler_t handler);

int raise(int sig);
int kill(pid_t pid, int sig);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigpending(sigset_t *set);
int sigsuspend(const sigset_t *set);
int sigaltstack(const stack_t *ss, stack_t *oss);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigwait(const sigset_t *set, int *sig);
