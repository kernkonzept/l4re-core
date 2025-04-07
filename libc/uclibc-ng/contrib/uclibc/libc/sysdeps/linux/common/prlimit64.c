/*  Copyright (C) 2023 uClibc-ng
 *  An prlimit64() - get/set resource limits Linux specific syscall.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, see
 *  <http://www.gnu.org/licenses/>.
 */

#include <sys/resource.h>
#include <sysdep.h>

#if defined(__NR_prlimit64)

int
prlimit64 (__pid_t pid, enum __rlimit_resource resource,
	   const struct rlimit64 *new_rlimit, struct rlimit64 *old_rlimit)
{
	return INLINE_SYSCALL (prlimit64, 4, pid, resource, new_rlimit,
			       old_rlimit);
}

# if __WORDSIZE == 64 || defined (__USE_FILE_OFFSET64)
strong_alias_untyped(prlimit64, prlimit)
# endif

#endif