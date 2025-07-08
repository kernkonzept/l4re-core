/* stub for arch-specific syscall issues/specific implementations */

#ifndef _DL_SYSCALLS_H
#define _DL_SYSCALLS_H

#if defined(__VDSO_SUPPORT__) && !defined(UCLIBC_LDSO)

#include "../dl-vdso-calls.h"

static int __attribute__ ((used)) __arm_vdso_clock_gettime(clockid_t clock_id, struct timespec *tp);
static int __attribute__ ((used)) __arm_vdso_clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    return __generic_vdso_clock_gettime(clock_id, tp);
}

static int __attribute__ ((used)) __arm_vdso_gettimeofday(struct timeval *tv, __timezone_ptr_t tz);
static int __attribute__ ((used)) __arm_vdso_gettimeofday(struct timeval *tv, __timezone_ptr_t tz)
{
    return __generic_vdso_gettimeofday(tv, tz);
}

#define ARCH_VDSO_GETTIMEOFDAY(tv, tz)        __arm_vdso_gettimeofday(tv, tz)
#define ARCH_VDSO_CLOCK_GETTIME(clock_id, tp) __arm_vdso_clock_gettime(clock_id, tp)

#endif /* defined(__VDSO_SUPPORT__) && !defined(UCLIBC_LDSO) */

#endif /* _DL_SYSCALLS_H */

