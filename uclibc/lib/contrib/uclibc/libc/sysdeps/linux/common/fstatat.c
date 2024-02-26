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
#include <bits/uClibc_arch_features.h>

/* 64bit ports tend to favor newfstatat() */
#if __WORDSIZE == 64 && defined __NR_newfstatat
# define __NR_fstatat64 __NR_newfstatat
#endif

#if defined(__NR_fstatat64) && !defined(__UCLIBC_USE_TIME64__)
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
# endif /* __ARCH_HAS_DEPRECATED_SYSCALLS__ */
	return ret;
}
libc_hidden_def(fstatat)
#else

#if defined(__NR_statx) && defined __UCLIBC_HAVE_STATX__
#include <sys/sysmacros.h> // for makedev

int fstatat(int fd, const char *file, struct stat *buf, int flag)
{
	int ret;
	struct statx tmp;

	ret = INLINE_SYSCALL(statx, 5, fd, file, flag,
			     STATX_BASIC_STATS, &tmp);
	if (ret != 0)
		return ret;

	*buf = (struct stat) {
		.st_dev = makedev(tmp.stx_dev_major, tmp.stx_dev_minor),
		.st_ino = tmp.stx_ino,
		.st_mode = tmp.stx_mode,
		.st_nlink = tmp.stx_nlink,
		.st_uid = tmp.stx_uid,
		.st_gid = tmp.stx_gid,
		.st_rdev = makedev(tmp.stx_rdev_major, tmp.stx_rdev_minor),
		.st_size = tmp.stx_size,
		.st_blksize = tmp.stx_blksize,
		.st_blocks = tmp.stx_blocks,
		.st_atim.tv_sec = tmp.stx_atime.tv_sec,
		.st_atim.tv_nsec = tmp.stx_atime.tv_nsec,
		.st_mtim.tv_sec = tmp.stx_mtime.tv_sec,
		.st_mtim.tv_nsec = tmp.stx_mtime.tv_nsec,
		.st_ctim.tv_sec = tmp.stx_ctime.tv_sec,
		.st_ctim.tv_nsec = tmp.stx_ctime.tv_nsec,
	};

	return ret;
}
libc_hidden_def(fstatat)

#endif

/* should add emulation with fstat() and /proc/self/fd/ ... */
#endif
