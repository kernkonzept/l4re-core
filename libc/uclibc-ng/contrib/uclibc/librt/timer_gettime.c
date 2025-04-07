/*
 * timer_gettime.c - get the timer value.
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#if defined(__NR_timer_gettime) || defined(__NR_timer_gettime64)

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_timer_gettime64)
#define __NR___syscall_timer_gettime __NR_timer_gettime64
#else
#define __NR___syscall_timer_gettime __NR_timer_gettime
#endif
static __inline__ _syscall2(int, __syscall_timer_gettime, kernel_timer_t, ktimerid,
			void *, value);

/* Get the amount of time left on a timer */
int timer_gettime(timer_t timerid, struct itimerspec *value)
{
	struct timer *kt = (struct timer *)timerid;

	/* Get timeout from the kernel */
	return __syscall_timer_gettime(kt->ktimerid, value);
}

#endif
