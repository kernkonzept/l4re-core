/*
 * getrlimit() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/resource.h>
#include <bits/wordsize.h>
#include <stddef.h> // needed for NULL to be defined

/* Only wrap getrlimit if the new ugetrlimit is not present and getrlimit sucks */

#if defined(__NR_ugetrlimit)

/* just call ugetrlimit() */
# define __NR___syscall_ugetrlimit __NR_ugetrlimit
static __always_inline
_syscall2(int, __syscall_ugetrlimit, enum __rlimit_resource, resource,
          struct rlimit *, rlim)
int getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits)
{
	return __syscall_ugetrlimit(resource, rlimits);
}
libc_hidden_def(getrlimit)

#elif defined(__NR_prlimit64)
/* Use prlimit64 if present, the prlimit64 syscall is free from a back 
   compatibility stuff for an old getrlimit */

# if __WORDSIZE == 32 && !defined(__USE_FILE_OFFSET64)
/* If struct rlimit has 64-bit fields (if __WORDSIZE == 64 or __USE_FILE_OFFSET64
   is defined), then use getrlimit as an alias to getrlimit64, see getrlimit64.c */
int getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits)
{
	struct rlimit64 rlimits64;
	int res = INLINE_SYSCALL (prlimit64, 4, 0, resource, NULL, &rlimits64);

	if (res == 0) {
		/* If the syscall succeeds but the values do not fit into a
		   rlimit structure set EOVERFLOW errno and retrun -1. */
		rlimits->rlim_cur = rlimits64.rlim_cur;
 		if (rlimits64.rlim_cur != rlimits->rlim_cur) {
			if (rlimits64.rlim_cur != RLIM64_INFINITY) {
				__set_errno(EOVERFLOW);
				return -1;
			}
			rlimits->rlim_cur = RLIM_INFINITY;
		}

		rlimits->rlim_max = rlimits64.rlim_max;
		if (rlimits64.rlim_max != rlimits->rlim_max) {
			if (rlimits64.rlim_max != RLIM64_INFINITY) {
				__set_errno(EOVERFLOW);
				return -1;
			}
			rlimits->rlim_max = RLIM_INFINITY;
		}
	}
	return res;
}
libc_hidden_def(getrlimit)
# endif

#else

# if !defined(__UCLIBC_HANDLE_OLDER_RLIMIT__)

/* We don't need to wrap getrlimit() */
_syscall2(int, getrlimit, __rlimit_resource_t, resource,
	  struct rlimit *, rlim)

# else

/* we have to handle old style getrlimit() */
#  define __NR___syscall_getrlimit __NR_getrlimit
static __always_inline
_syscall2(int, __syscall_getrlimit, int, resource, struct rlimit *, rlim)

int getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits)
{
	int result;

	result = __syscall_getrlimit(resource, rlimits);

	if (result == -1)
		return result;

	/* We might have to correct the limits values.  Since the old values
	 * were signed the infinity value is too small.  */
	if (rlimits->rlim_cur == RLIM_INFINITY >> 1)
		rlimits->rlim_cur = RLIM_INFINITY;
	if (rlimits->rlim_max == RLIM_INFINITY >> 1)
		rlimits->rlim_max = RLIM_INFINITY;
	return result;
}
# endif

libc_hidden_def(getrlimit)
#endif
