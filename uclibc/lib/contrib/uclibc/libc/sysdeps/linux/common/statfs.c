/* vi: set sw=4 ts=4: */
/*
 * statfs() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <string.h>
#include <sys/param.h>
#include <sys/vfs.h>

extern __typeof(statfs) __libc_statfs attribute_hidden;

#if defined __NR_statfs64 && !defined __NR_statfs

int __libc_statfs(const char *path, struct statfs *buf)
{
	int err = INLINE_SYSCALL(statfs64, 3, path, sizeof(*buf), buf);

	if (err == 0) {
		/* Did we overflow? */
		if (buf->__pad1 || buf->__pad2 || buf->__pad3 ||
		    buf->__pad4 || buf->__pad5) {
			__set_errno(EOVERFLOW);
			return -1;
		}
	}

	return err;
}
# if defined __UCLIBC_LINUX_SPECIFIC__ || defined __UCLIBC_HAS_THREADS_NATIVE__
/* statfs is used by NPTL, so it must exported in case */
weak_alias(__libc_statfs, statfs)
# endif

/* For systems which have both, prefer the old one */
#else

# define __NR___libc_statfs __NR_statfs
_syscall2(int, __libc_statfs, const char *, path, struct statfs *, buf)

# if defined __UCLIBC_LINUX_SPECIFIC__ || defined __UCLIBC_HAS_THREADS_NATIVE__
/* statfs is used by NPTL, so it must exported in case */
weak_alias(__libc_statfs, statfs)
# endif

#endif
libc_hidden_def(statfs)
