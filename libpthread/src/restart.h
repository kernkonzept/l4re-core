/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

//#include <signal.h>
//#include <kernel-features.h>
#include <l4/sys/thread.h>
#include <stdlib.h>
#include <stdio.h>

#include <l4/sys/types.h>
#include <l4/sys/semaphore.h>

/* Primitives for controlling thread execution */

static __inline__ void restart(pthread_descr th)
{
  l4_umword_t ipc_error = l4_ipc_error(l4_semaphore_up(th->p_thsem_cap), l4_utcb());
  if (L4_UNLIKELY(ipc_error))
    {
      dprintf(STDERR_FILENO, "libpthread: error %s in restart()/l4_semaphore_up()",
              l4sys_errtostr(l4_ipc_to_errno(ipc_error)));
      abort();
    }
}

static __inline__ void suspend(pthread_descr self)
{
  l4_ret_t error = l4_error(l4_semaphore_down(self->p_thsem_cap, L4_IPC_NEVER));
  if (L4_UNLIKELY(error < 0))
    {
      dprintf(STDERR_FILENO, "libpthread: error %s in suspend()/l4_semaphore_down()",
              l4sys_errtostr(error));
      abort();
    }
}

/**
 * Wait until restart() or until `abstime` has passed.
 *
 * \retval 0  The deadline passed.
 * \retval 1  The thread was restarted.
 */
static __inline__
int timedsuspend(pthread_descr self, const struct timespec *abstime)
{
  extern uint64_t __attribute__((weak)) __libc_l4_rt_clock_offset;

  if (abstime->tv_sec < 0 || abstime->tv_nsec < 0)
    return 0;

  uint64_t sec = (uint64_t)abstime->tv_sec;
  uint64_t usec = (uint64_t)abstime->tv_nsec / 1000;

  uint64_t clock;
  if (sec >= ~0ULL / 1000000ULL)
    clock = ~0ULL;
  else
    clock = sec * 1000000ULL + usec;

  if (&__libc_l4_rt_clock_offset)
    {
      if (clock < __libc_l4_rt_clock_offset)
        return 0;
      clock -= __libc_l4_rt_clock_offset;
    }

  l4_timeout_t timeout = L4_IPC_NEVER;
  l4_rcv_timeout(l4_timeout_abs_u(clock, 4, l4_utcb()), &timeout);
  l4_msgtag_t res = l4_semaphore_down(self->p_thsem_cap, timeout);
  if (l4_error(res) == l4_ipc_to_errno(L4_IPC_RETIMEOUT))
    return 0;
  return 1;
}
