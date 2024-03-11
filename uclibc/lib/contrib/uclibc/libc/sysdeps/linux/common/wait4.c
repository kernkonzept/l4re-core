/*
 * wait4() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/resource.h>

#if defined(__NR_wait4)
# define __NR___syscall_wait4 __NR_wait4
static __always_inline _syscall4(int, __syscall_wait4, __kernel_pid_t, pid,
				 int *, status, int, opts, struct rusage *, rusage)

pid_t __wait4_nocancel(pid_t pid, int *status, int opts, struct rusage *rusage)
{
#if defined(__UCLIBC_USE_TIME64__)
	char *arg_rusage = rusage ? (char *)&rusage->ru_maxrss - 4 * sizeof(__S32_TYPE) : 0;
	int __ret = __syscall_wait4(pid, status, opts, (struct rusage *)arg_rusage);
	if (__ret > 0 && rusage) {
		__S32_TYPE __rusage[4];
		memcpy(__rusage, arg_rusage, 4 * sizeof(__S32_TYPE));
		struct timeval tv_utime = {.tv_sec = __rusage[0], .tv_usec = __rusage[1]};
		struct timeval tv_stime = {.tv_sec = __rusage[2], .tv_usec = __rusage[2]};
		rusage->ru_utime = tv_utime;
		rusage->ru_stime = tv_stime;
	}
	return __ret;
#else
	return __syscall_wait4(pid, status, opts, rusage);
#endif
}

#else
pid_t __wait4_nocancel(pid_t pid, int *status, int opts, struct rusage *rusage)
{
	idtype_t type;
	int __res;
	siginfo_t info;

	info.si_pid = 0;

	if (pid < -1) {
		type = P_PGID;
		pid = -pid;
	} else if (pid == -1) {
		type = P_ALL;
	} else if (pid == 0) {
		type = P_PGID;
	} else {
		type = P_PID;
	}

	__res = INLINE_SYSCALL(waitid, 5, type, pid, &info, opts|WEXITED, rusage);

	if ( __res < 0 )
		return __res;

	if (info.si_pid && status) {
			int sw = 0;
			switch (info.si_code) {
			case CLD_CONTINUED:
				sw = 0xffff;
				break;
			case CLD_DUMPED:
				sw = (info.si_status & 0x7f) | 0x80;
				break;
			case CLD_EXITED:
				sw = (info.si_status & 0xff) << 8;
				break;
			case CLD_KILLED:
				sw = info.si_status & 0x7f;
				break;
			case CLD_STOPPED:
			case CLD_TRAPPED:
				sw = (info.si_status << 8) + 0x7f;
				break;
			}
			*status = sw;
	}

	return info.si_pid;
}
#endif

#ifdef __USE_BSD
strong_alias(__wait4_nocancel,wait4)
#endif
