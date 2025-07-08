/* Machine-dependent pthreads configuration and inline functions.
   ARM version.
   Copyright (C) 1997, 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

/* This will not work on ARM1 or ARM2 because SWP is lacking on those
   machines.  Unfortunately we have no way to detect this at compile
   time; let's hope nobody tries to use one.  */

/* Spinlock implementation; required.  */
L4_INLINE long int
testandset (int *spinlock)
{
  return __atomic_exchange_n(spinlock, 1, __ATOMIC_ACQUIRE);
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("sp");

/* Compare-and-swap for semaphores.  It's always available on i686.  */
#define HAS_COMPARE_AND_SWAP

L4_INLINE int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  return __atomic_compare_exchange_n(p, &oldval, newval, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#endif /* pt-machine.h */
