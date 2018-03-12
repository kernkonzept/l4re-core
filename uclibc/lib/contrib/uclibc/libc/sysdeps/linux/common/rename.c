/* vi: set sw=4 ts=4: */
/*
 * rename() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>

#if defined __NR_renameat && !defined __NR_rename
# include <fcntl.h>
int rename(const char *oldpath, const char *newpath)
{
	return renameat(AT_FDCWD, oldpath, AT_FDCWD, newpath);
}
#else
_syscall2(int, rename, const char *, oldpath, const char *, newpath)
#endif
