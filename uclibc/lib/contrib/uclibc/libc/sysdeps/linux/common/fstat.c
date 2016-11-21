/* vi: set sw=4 ts=4: */
/*
 * fstat() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>
#include "xstatconv.h"

#if defined __NR_fstat64 && !defined __NR_fstat
int fstat(int fd, struct stat *buf)
{
	int result = INLINE_SYSCALL(fstat64, 2, fd, buf);
	if (result == 0) {
		/* Did we overflow? */
		if (buf->__pad1 || buf->__pad2 || buf->__pad3
		    || buf->__pad4 || buf->__pad5
		    || buf->__pad6 || buf->__pad7) {
			__set_errno(EOVERFLOW);
			return -1;
		}
	}
	return result;
}
libc_hidden_def(fstat)

#elif defined __NR_fstat
int fstat(int fd, struct stat *buf)
{
	int result;
# ifdef __NR_fstat64
	/* normal stat call has limited values for various stat elements
	 * e.g. uid device major/minor etc.
	 * so we use 64 variant if available
	 * in order to get newer versions of stat elements
	 */
	struct kernel_stat64 kbuf;
	result = INLINE_SYSCALL(fstat64, 2, fd, &kbuf);
	if (result == 0) {
		__xstat32_conv(&kbuf, buf);
	}
# else
	struct kernel_stat kbuf;

	result = INLINE_SYSCALL(fstat, 2, fd, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
# endif
	return result;
}
libc_hidden_def(fstat)

# if ! defined __NR_fstat64
strong_alias_untyped(fstat,fstat64)
libc_hidden_def(fstat64)
# endif

#endif
