/*
 * fstatat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>
#include "xstatconv.h"

/* 64bit ports tend to favor newfstatat() */
#if __WORDSIZE == 64 && defined __NR_newfstatat
# define __NR_fstatat64 __NR_newfstatat
#endif

#ifdef __NR_fstatat64
int fstatat(int fd, const char *file, struct stat *buf, int flag)
{
	int ret;
# ifdef __ARCH_HAS_DEPRECATED_SYSCALLS__
	struct kernel_stat64 kbuf;
	ret = INLINE_SYSCALL(fstatat64, 4, fd, file, &kbuf, flag);
	if (ret == 0)
		__xstat32_conv(&kbuf, buf);
# else
	ret = INLINE_SYSCALL(fstatat64, 4, fd, file, buf, flag);
	if (ret == 0) {
		/* Did we overflow */
		if (buf->__pad1 || buf->__pad2 || buf->__pad3
		    || buf->__pad4 || buf->__pad5 || buf->__pad6
		    || buf->__pad7) {
			__set_errno(EOVERFLOW);
			return -1;
		}
	}
# endif /* __ARCH_HAS_DEPRECATED_SYSCALLS__ */
	return ret;
}
libc_hidden_def(fstatat)
#else
/* should add emulation with fstat() and /proc/self/fd/ ... */
#endif
