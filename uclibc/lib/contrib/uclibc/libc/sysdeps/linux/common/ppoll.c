/* Copyright (C) 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2006.

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

#include <sys/syscall.h>

#if (defined __NR_ppoll || defined(__NR_ppoll_time64)) && defined __UCLIBC_LINUX_SPECIFIC__ && defined __USE_GNU

#define __need_NULL
#include <stddef.h>
#include <signal.h>
#include <sys/poll.h>
#include <cancel.h>

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

static int
__NC(ppoll)(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
	    const sigset_t *sigmask)
{
	/* The Linux kernel can in some situations update the timeout value.
	   We do not want that so use a local variable.  */
	struct timespec tval;
	if (timeout != NULL) {
		tval = *timeout;
		timeout = &tval;
	}
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_ppoll_time64)
	return INLINE_SYSCALL(ppoll_time64, 5, fds, nfds, TO_TS64_P(timeout), sigmask, __SYSCALL_SIGSET_T_SIZE);
#else
	return INLINE_SYSCALL(ppoll, 5, fds, nfds, timeout, sigmask, __SYSCALL_SIGSET_T_SIZE);
#endif
}

CANCELLABLE_SYSCALL(int, ppoll, (struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
				 const sigset_t *sigmask),
		    (fds, nfds, timeout, sigmask))
#endif
