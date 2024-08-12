/* process_vm_readv - Linux specific syscall.
   Copyright (C) 2020-2024 Free Software Foundation, Inc.

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
   <https://www.gnu.org/licenses/>.  */

#include <sys/uio.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_process_vm_readv
ssize_t
process_vm_readv (pid_t pid, const struct iovec *local_iov,
		  unsigned long int liovcnt,
		  const struct iovec *remote_iov,
		  unsigned long int riovcnt, unsigned long int flags)
{
  return INLINE_SYSCALL (process_vm_readv, 6, pid, local_iov,
			      liovcnt, remote_iov, riovcnt, flags);
}
#endif
