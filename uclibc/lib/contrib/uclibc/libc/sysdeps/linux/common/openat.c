/*
 * openat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>

#ifdef __NR_openat
# define __NR___syscall_openat __NR_openat
static __inline__ _syscall4(int, __syscall_openat, int, fd, const char *, file, int, oflag, mode_t, mode)
strong_alias_untyped(__syscall_openat,openat)
libc_hidden_def(openat)
#else
/* should add emulation with open() and /proc/self/fd/ ... */
#endif
