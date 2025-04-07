/*
 * settimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __USE_BSD
# include <sys/time.h>
# include <time.h>

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	if (!tv)
		return 0;

	struct timespec __ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * 1000
	};

	return clock_settime(CLOCK_REALTIME, &__ts);
}

# elif defined __USE_SVID && defined __NR_stime
#  define __need_NULL
#  include <stddef.h>
#  include <errno.h>
#  include <time.h>
int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	time_t when;

	if (tv == NULL) {
		__set_errno(EINVAL);
		return -1;
	}

	if (tz != NULL || tv->tv_usec % 1000000 != 0) {
		__set_errno(ENOSYS);
		return -1;
	}

	when = tv->tv_sec + (tv->tv_usec / 1000000);
	return stime(&when);
}
# endif
# if defined __NR_settimeofday || defined(__UCLIBC_USE_TIME64__) || (defined __USE_SVID && defined __NR_stime)
libc_hidden_def(settimeofday)
# endif
