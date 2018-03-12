/* vi: set sw=4 ts=4: */
/*
 * fork() for Xtensa uClibc
 *
 * Copyright (C) 2007 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */


/* Xtensa doesn't provide a 'fork' system call, so we use 'clone'.  */
#include <sys/syscall.h>

#if defined __NR_clone && defined __ARCH_USE_MMU__
# include <unistd.h>
# include <signal.h>
# include <cancel.h>

pid_t fork(void)
{
	return (pid_t) INLINE_SYSCALL(clone, 2, SIGCHLD, 0);
}
lt_strong_alias(fork)
lt_libc_hidden(fork)
#endif
