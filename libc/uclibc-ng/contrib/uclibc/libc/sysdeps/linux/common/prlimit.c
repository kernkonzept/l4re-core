/* Get/set resource limits.  Linux specific syscall.
   Copyright (C) 2021-2022 Free Software Foundation, Inc.

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

#include <sys/resource.h>
#include <sysdep.h>
#include <stddef.h> // needed for NULL to be defined

#if defined(__NR_prlimit64) && __WORDSIZE == 32 && !defined(__USE_FILE_OFFSET64)
int
prlimit (__pid_t pid, enum __rlimit_resource resource,
	 const struct rlimit *new_rlimit, struct rlimit *old_rlimit)
{
	struct rlimit64 new_rlimit64;
	struct rlimit64 *new_rlimit64_ptr = NULL;
	struct rlimit64 old_rlimit64;
	struct rlimit64 *old_rlimit64_ptr = (old_rlimit != NULL ? &old_rlimit64 : NULL);
	int res;

	if (new_rlimit != NULL) {
		if (new_rlimit->rlim_cur == RLIM_INFINITY)
			new_rlimit64.rlim_cur = RLIM64_INFINITY;
		else
			new_rlimit64.rlim_cur = new_rlimit->rlim_cur;
		if (new_rlimit->rlim_max == RLIM_INFINITY)
			new_rlimit64.rlim_max = RLIM64_INFINITY;
		else
			new_rlimit64.rlim_max = new_rlimit->rlim_max;
		new_rlimit64_ptr = &new_rlimit64;
	}

	res = INLINE_SYSCALL (prlimit64, 4, pid, resource, new_rlimit64_ptr,
			      old_rlimit64_ptr);

	if (res == 0 && old_rlimit != NULL) {
		/* If the syscall succeeds but the values do not fit into a
		   rlimit structure set EOVERFLOW errno and retrun -1.
		   With current Linux implementation of the prlimit64 syscall,
		   overflow can't happen. An extra condition has been added to get 
		   the same behavior as in glibc for future potential overflows. */
		old_rlimit->rlim_cur = old_rlimit64.rlim_cur;
		if (old_rlimit64.rlim_cur != old_rlimit->rlim_cur) {
			if (new_rlimit == NULL && 
			    old_rlimit64.rlim_cur != RLIM64_INFINITY) {
				__set_errno(EOVERFLOW);
				return -1;
			}
			old_rlimit->rlim_cur = RLIM_INFINITY;
		}
		old_rlimit->rlim_max = old_rlimit64.rlim_max;
		if (old_rlimit64.rlim_max != old_rlimit->rlim_max) {
			if (new_rlimit == NULL &&
			    old_rlimit64.rlim_max != RLIM64_INFINITY) {
				__set_errno(EOVERFLOW);
				return -1;
			}
			old_rlimit->rlim_max = RLIM_INFINITY;
		}
	}

	return res;
}
#endif
