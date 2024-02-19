/*
 * timer_settime.c - set the timer.
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#if defined(__NR_timer_settime) || defined(__NR_timer_settime64)

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_timer_settime64)
#define __NR___syscall_timer_settime __NR_timer_settime64
#else
#define __NR___syscall_timer_settime __NR_timer_settime
#endif

static __inline__ _syscall4(int, __syscall_timer_settime, kernel_timer_t, ktimerid,
			int, flags, const void *, value, void *, ovalue);

/* Set the expiration time for a timer */
int timer_settime(timer_t timerid, int flags, const struct itimerspec *value,
		  struct itimerspec *ovalue)
{
	struct timer *kt = (struct timer *)timerid;

	/* Set timeout */
	return __syscall_timer_settime(kt->ktimerid, flags, value, ovalue);
}

#endif
