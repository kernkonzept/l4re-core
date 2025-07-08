/* POSIX spinlock implementation.  SPARC32 version.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <pthread.h>
#include "internals.h"

/* This implementation is similar to the one used in the Linux kernel.  */
int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  __asm__ __volatile__
    ("1: ldstub [%0], %%g2\n"
     "   orcc   %%g2, 0x0, %%g0\n"
     "   bne,a  2f\n"
     "   ldub   [%0], %%g2\n"
     ".subsection 2\n"
     "2: orcc   %%g2, 0x0, %%g0\n"
     "   bne,a  2b\n"
     "   ldub   [%0], %%g2\n"
     "   b,a    1b\n"
     ".previous"
     : /* no outputs */
     : "r" (lock)
     : "g2", "memory", "cc");
  return 0;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  int result;
  __asm__ __volatile__
    ("ldstub [%1], %0"
     : "=r" (result)
     : "r" (lock)
     : "memory");
  return result == 0 ? 0 : EBUSY;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)


int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  *lock = 0;
  return 0;
}
weak_alias (__pthread_spin_unlock, pthread_spin_unlock)


int
__pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  *lock = 0;
  return 0;
}
weak_alias (__pthread_spin_init, pthread_spin_init)


int
__pthread_spin_destroy (pthread_spinlock_t *lock)
{
  /* Nothing to do.  */
  return 0;
}
weak_alias (__pthread_spin_destroy, pthread_spin_destroy)
