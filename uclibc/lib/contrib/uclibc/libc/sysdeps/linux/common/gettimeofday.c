/*
 * gettimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>

#ifdef __VDSO_SUPPORT__
#include "ldso.h"
#endif

int gettimeofday(struct timeval * tv, __timezone_ptr_t tz) {
    if (!tv)
        return 0;
#if defined(__VDSO_SUPPORT__) && defined(ARCH_VDSO_GETTIMEOFDAY)
    return ARCH_VDSO_GETTIMEOFDAY(tv, tz);
#else
    struct timespec __ts;
    int __ret = clock_gettime(CLOCK_REALTIME, &__ts);
    tv->tv_sec = __ts.tv_sec;
    tv->tv_usec = (suseconds_t)__ts.tv_nsec / 1000;
    return __ret;
#endif
}


libc_hidden_def(gettimeofday)
