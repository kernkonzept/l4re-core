/*
 * flock() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifdef NOT_FOR_L4
#include <sys/syscall.h>
#else
#include <errno.h>
#endif
#include <sys/file.h>

#ifdef NOT_FOR_L4
#define __NR___syscall_flock __NR_flock
static __inline__ _syscall2(int, __syscall_flock, int, fd, int, operation)
#endif

int flock(int fd, int operation)
{
#ifdef NOT_FOR_L4
	return (__syscall_flock(fd, operation));
#else
        errno = EINVAL;
        return -1;
#endif
}
