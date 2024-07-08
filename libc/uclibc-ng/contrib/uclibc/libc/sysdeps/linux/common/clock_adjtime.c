/*
 * clock_adjtime() for uClibc
 *
 * Copyright (C) 2005 by Peter Kjellerstedt <pkj@axis.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/timex.h>

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_clock_adjtime64)
_syscall2_64(int, clock_adjtime, clockid_t, clock_id, struct timex*, ntx)
#elif defined(__NR_clock_adjtime)
_syscall2(int, clock_adjtime, clockid_t, clock_id, struct timex*, ntx)
#else
#warning "clock_adjtime syscall is not defined!"
#endif
