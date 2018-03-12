/* vi: set sw=4 ts=4: */
/*
 * ftruncate() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#if defined __NR_ftruncate64 && !defined __NR_ftruncate
# include <endian.h>
# include <stdint.h>
int ftruncate(int fd, __off_t length)
{
# if defined __UCLIBC_HAS_LFS__
	return ftruncate64(fd, length);
# elif __WORDSIZE == 32
	return INLINE_SYSCALL(ftruncate64, 3, fd, OFF_HI_LO(length));
# endif
}
libc_hidden_def(ftruncate);

#else
_syscall2(int, ftruncate, int, fd, __off_t, length)
libc_hidden_def(ftruncate)
#endif
