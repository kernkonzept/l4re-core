/* Copyright (C) 1999, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <alloca.h>
#include <sys/syscall.h>

extern int __syscall_execve(const char *file,
			char *const *argv,
			char *const *envp);
extern void __pthread_kill_other_threads_np(void);
weak_extern(__pthread_kill_other_threads_np)

int
__execve(const char *file, char *const argv[], char *const envp[])
{
	/* If this is a threaded application kill all other threads.  */
	if (__pthread_kill_other_threads_np)
		__pthread_kill_other_threads_np();
	return INLINE_SYSCALL(execve, 3, file, argv, envp);
}
weak_alias(__execve, execve)
