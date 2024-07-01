/*
 * setrlimit() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/resource.h>
#include <bits/wordsize.h>
#include <stddef.h> // needed for NULL to be defined

/* Only wrap setrlimit if the new usetrlimit is not present and setrlimit sucks */

#if defined(__NR_usetrlimit)
/* just call usetrlimit() */
# define __NR___syscall_usetrlimit __NR_usetrlimit
static __always_inline
_syscall2(int, __syscall_usetrlimit, enum __rlimit_resource, resource,
          const struct rlimit *, rlim)
int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlimits)
{
	return __syscall_usetrlimit(resource, rlimits);
}
libc_hidden_def(setrlimit)

#elif defined(__NR_prlimit64)

/* Use prlimit64 if present, the prlimit64 syscall is free from a back
   compatibility stuff for setrlimit */

 # if __WORDSIZE == 32 && !defined(__USE_FILE_OFFSET64)
/* If struct rlimit has 64-bit fields (if __WORDSIZE == 64 or __USE_FILE_OFFSET64
   is defined), then use setrlimit as an alias to setrlimit64, see setrlimit64.c */
int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlimits)
{
	struct rlimit64 rlimits64;

	if (rlimits->rlim_cur == RLIM_INFINITY)
		rlimits64.rlim_cur = RLIM64_INFINITY;
	else
		rlimits64.rlim_cur = rlimits->rlim_cur;
	if (rlimits->rlim_max == RLIM_INFINITY)
		rlimits64.rlim_max = RLIM64_INFINITY;
	else
		rlimits64.rlim_max = rlimits->rlim_max;

	return INLINE_SYSCALL (prlimit64, 4, 0, resource, &rlimits64, NULL);
}
libc_hidden_def(setrlimit)
# endif

#else

# if !defined(__UCLIBC_HANDLE_OLDER_RLIMIT__)

/* We don't need to wrap setrlimit() */
_syscall2(int, setrlimit, __rlimit_resource_t, resource,
		const struct rlimit *, rlim)

# else

#  define __need_NULL
#  include <stddef.h>
#  include <errno.h>
#  include <sys/param.h>

/* we have to handle old style setrlimit() */
#  define __NR___syscall_setrlimit __NR_setrlimit
static __always_inline
_syscall2(int, __syscall_setrlimit, int, resource, const struct rlimit *, rlim)

int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlimits)
{
	struct rlimit rlimits_small;

	if (rlimits == NULL) {
		__set_errno(EINVAL);
		return -1;
	}

	/* We might have to correct the limits values.  Since the old values
	 * were signed the new values might be too large.  */
	rlimits_small.rlim_cur = MIN((unsigned long int) rlimits->rlim_cur,
								  RLIM_INFINITY >> 1);
	rlimits_small.rlim_max = MIN((unsigned long int) rlimits->rlim_max,
								  RLIM_INFINITY >> 1);
	return __syscall_setrlimit(resource, &rlimits_small);
}
# endif

libc_hidden_def(setrlimit)
#endif
