/*
 * utimensat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#if defined(__NR_utimensat) || defined(__NR_utimensat_time64)
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_utimensat_time64)
_syscall4_time64(int, utimensat, int, fd, const char *, path, const struct timespec *, times, int, flags)
#else
_syscall4(int, utimensat, int, fd, const char *, path, const struct timespec *, times, int, flags)
#endif
libc_hidden_def(utimensat)
#else
/* should add emulation with utimens() and /proc/self/fd/ ... */
#endif

