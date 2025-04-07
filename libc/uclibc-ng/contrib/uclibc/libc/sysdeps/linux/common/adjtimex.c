/*
 * adjtimex() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/timex.h>

#if defined(__NR_adjtimex)
_syscall1(int, adjtimex, struct timex *, buf)
#else
#include <time.h>
int adjtimex(struct timex *buf)
{
    return clock_adjtime(CLOCK_REALTIME, buf);
}
#endif

libc_hidden_def(adjtimex)
weak_alias(adjtimex,__adjtimex)
#if defined __UCLIBC_NTP_LEGACY__
strong_alias(adjtimex,ntp_adjtime)
#endif
