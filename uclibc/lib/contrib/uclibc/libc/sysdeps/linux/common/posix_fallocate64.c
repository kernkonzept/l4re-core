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

# if __WORDSIZE == 64
/* Can use normal posix_fallocate() */
# elif __WORDSIZE == 32
int posix_fallocate64(int fd, __off64_t offset, __off64_t len)
{
	int ret;
	INTERNAL_SYSCALL_DECL(err);
	ret = (int) (INTERNAL_SYSCALL(fallocate, err, 6, fd, 0,
		OFF64_HI_LO (offset), OFF64_HI_LO (len)));
    if (unlikely(INTERNAL_SYSCALL_ERROR_P (ret, err)))
      return INTERNAL_SYSCALL_ERRNO (ret, err);
    return 0;
}
# else
# error your machine is neither 32 bit or 64 bit ... it must be magical
# endif
#endif
