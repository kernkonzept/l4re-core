/* Copyright (C) 2003-2018 Free Software Foundation, Inc.

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

#include <time.h>
#include <errno.h>
#include <cancel.h>
#include <sys/syscall.h>

#include "kernel-posix-cpu-timers.h"

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

/* We can simply use the syscall.  The CPU clocks are not supported
   with this function.  */
int
clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
		   struct timespec *rem)
{
  INTERNAL_SYSCALL_DECL (err);
  int r;

  if (clock_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;
  if (clock_id == CLOCK_PROCESS_CPUTIME_ID)
    clock_id = MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED);

  if (SINGLE_THREAD_P) {
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_clock_nanosleep_time64)
    struct __ts64_struct __req, __rem;
    __req.tv_sec = req->tv_sec;
    __req.tv_nsec = req->tv_nsec;
    r = INTERNAL_SYSCALL (clock_nanosleep_time64, err, 4, clock_id, flags, &__req, &__rem);
    if (rem) {
      rem->tv_sec = (time_t) __rem.tv_sec;
      rem->tv_nsec = __rem.tv_nsec;
    }
#else
    r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags, req, rem);
#endif
  }
  else
    {
#ifdef __NEW_THREADS
      int oldstate = LIBC_CANCEL_ASYNC ();
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_clock_nanosleep_time64)
      struct __ts64_struct __req, __rem;
      __req.tv_sec = req->tv_sec;
      __req.tv_nsec = req->tv_nsec;
      r = INTERNAL_SYSCALL (clock_nanosleep_time64, err, 4, clock_id, flags, &__req, &__rem);
      if (rem) {
        rem->tv_sec = (time_t) __rem.tv_sec;
        rem->tv_nsec = __rem.tv_nsec;
      }
#else
      r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags, req,
			    rem);
#endif
      LIBC_CANCEL_RESET (oldstate);
#endif
    }

  return (INTERNAL_SYSCALL_ERROR_P (r, err)
	  ? INTERNAL_SYSCALL_ERRNO (r, err) : 0);
}
