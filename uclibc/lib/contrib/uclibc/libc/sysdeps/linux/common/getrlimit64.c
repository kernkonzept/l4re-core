/* Copyright (C) 1991, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <_lfs_64.h>
#include <bits/wordsize.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <stddef.h> // needed for NULL to be defined


#if defined(__NR_prlimit64)

/* the regular prlimit64 will work just fine for 64-bit users */
int getrlimit64 (__rlimit_resource_t resource, struct rlimit64 *rlimits)
{
	return INLINE_SYSCALL (prlimit64, 4, 0, resource, NULL, rlimits);
}

# if !defined(__NR_ugetrlimit) && (__WORDSIZE == 64 || defined (__USE_FILE_OFFSET64))
/* If getrlimit is not implemented through the __NR_ugetrlimit and size of
   rlimit_t == rlimit64_t then use getrlimit as an alias to getrlimit64 */
strong_alias_untyped(getrlimit64, getrlimit)
libc_hidden_def(getrlimit)
# endif

#else

/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  
   The regular getrlimit will work just fine for 64-bit users */
int getrlimit64 (__rlimit_resource_t resource, struct rlimit64 *rlimits)
{
    struct rlimit rlimits32;

    if (getrlimit (resource, &rlimits32) < 0)
	return -1;

    if (rlimits32.rlim_cur == RLIM_INFINITY)
	rlimits->rlim_cur = RLIM64_INFINITY;
    else
	rlimits->rlim_cur = rlimits32.rlim_cur;
    if (rlimits32.rlim_max == RLIM_INFINITY)
	rlimits->rlim_max = RLIM64_INFINITY;
    else
	rlimits->rlim_max = rlimits32.rlim_max;

    return 0;
}
#endif

