/* vi: set sw=4 ts=4: */
/*
 * posix_fallocate() for uClibc
 * http://www.opengroup.org/onlinepubs/9699919799/functions/posix_fallocate.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>
#include <bits/kernel-features.h>
#include <stdint.h>

#if defined __NR_fallocate
extern __typeof(fallocate) __libc_fallocate attribute_hidden;
int posix_fallocate(int fd, __off_t offset, __off_t len)
{
	return __libc_fallocate(fd, 0, offset, len);
}
# if defined __UCLIBC_HAS_LFS__ && __WORDSIZE == 64
strong_alias(posix_fallocate,posix_fallocate64)
# endif
#endif
