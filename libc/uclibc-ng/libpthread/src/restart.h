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

#include <l4/sys/types.h>
#include <l4/sys/semaphore.h>

/* Primitives for controlling thread execution */

static __inline__ void restart(pthread_descr th)
{
  l4_semaphore_up(th->p_thsem_cap);
}

static __inline__ void suspend(pthread_descr self)
{
  l4_semaphore_down(self->p_thsem_cap, L4_IPC_NEVER);
}

static __inline__ int timedsuspend(pthread_descr self,
		const struct timespec *abstime)
{
  extern uint64_t __attribute__((weak)) __libc_l4_kclock_offset;
  uint64_t clock = abstime->tv_sec * 1000000ULL + abstime->tv_nsec / 1000;
  if (&__libc_l4_kclock_offset)
    clock -= __libc_l4_kclock_offset;
  l4_timeout_t timeout = L4_IPC_NEVER;
  l4_rcv_timeout(l4_timeout_abs_u(clock, 4, l4_utcb()), &timeout);
  l4_msgtag_t res = l4_semaphore_down(self->p_thsem_cap, timeout);
  if (l4_error(res) == -(L4_EIPC_LO + L4_IPC_RETIMEOUT))
    return 0;
  return 1;
}
