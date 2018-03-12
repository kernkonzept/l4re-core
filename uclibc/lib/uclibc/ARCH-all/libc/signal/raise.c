/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

int raise(int signo)
{
#ifdef l4___this_is_the_original_uclibc_needed_with_gnu_linux_compiler
	return kill(getpid(), signo);
#else
        __builtin_trap();
#endif
}
libc_hidden_def(raise)
