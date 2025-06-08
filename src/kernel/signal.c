#include "signal.h"
#include "debug.h"

sighandler_t signal(int signum, sighandler_t handler)
{
	log_debug("signal", "signum = %d, handler = %p", signum, handler);
	FUNC_NOT_IMPLEMENTED("signal", "signal");
	return handler; // Placeholder for signal function, should be implemented
}

int raise(int sig)
{
	log_debug("signal", "sig = %d", sig);
	FUNC_NOT_IMPLEMENTED("signal", "raise");
	return -1; // Placeholder for raise function, should be implemented
}
int kill(pid_t pid, int sig)
{
	log_debug("signal", "pid = %d, sig = %d", pid, sig);
	FUNC_NOT_IMPLEMENTED("signal", "kill");
	return -1; // Placeholder for kill function, should be implemented
}
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	log_debug("signal", "signum = %d, act = %p, oldact = %p", signum, act, oldact);
	FUNC_NOT_IMPLEMENTED("signal", "sigaction");
	return -1; // Placeholder for sigaction function, should be implemented
}
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	log_debug("signal", "how = %d, set = %p, oldset = %p", how, set, oldset);
	FUNC_NOT_IMPLEMENTED("signal", "sigprocmask");
	return -1; // Placeholder for sigprocmask function, should be implemented
}
int sigpending(sigset_t *set)
{
	log_debug("signal", "set = %p", set);
	FUNC_NOT_IMPLEMENTED("signal", "sigpending");
	return -1; // Placeholder for sigpending function, should be implemented
}
int sigsuspend(const sigset_t *set)
{
	log_debug("signal", "set = %p", set);
	FUNC_NOT_IMPLEMENTED("signal", "sigsuspend");
	return -1; // Placeholder for sigsuspend function, should be implemented
}
int sigaltstack(const stack_t *ss, stack_t *oss)
{
	log_debug("signal", "ss = %p, oss = %p", ss, oss);
	FUNC_NOT_IMPLEMENTED("signal", "sigaltstack");
	return -1; // Placeholder for sigaltstack function, should be implemented
}
int sigwait(const sigset_t *set, int *sig)
{
	log_debug("signal", "set = %p, sig = %p", set, sig);
	FUNC_NOT_IMPLEMENTED("signal", "sigwait");
	return -1; // Placeholder for sigwait function, should be implemented
}