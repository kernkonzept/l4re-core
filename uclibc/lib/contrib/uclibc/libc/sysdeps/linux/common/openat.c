/*
 * openat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>
#include <stdarg.h>

#ifdef __NR_openat
# define __NR___syscall_openat __NR_openat
static __inline__ _syscall4(int, __syscall_openat, int, fd, const char *, file, int, oflag, mode_t, mode)

int __openat(int fd, const char *file, int o_flag, ...)
{
	va_list ap;
	mode_t mode;

	va_start(ap, o_flag);
	mode = va_arg(ap, int);
	va_end(ap);
	return __syscall_openat(fd, file, o_flag, mode);
}

strong_alias_untyped(__openat,openat)
libc_hidden_def(openat)
#else
/* should add emulation with open() and /proc/self/fd/ ... */
#endif
