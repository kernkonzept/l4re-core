/* Machine-dependent pthreads configuration and inline functions.
   x86-64 version.
   Copyright (C) 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

# include <features.h>

#ifndef __ASSEMBLER__
# include <stddef.h>	/* For offsetof.  */
# include <stdlib.h>	/* For abort().  */
//# include <asm/prctl.h>


# ifndef PT_EI
#  define PT_EI __extern_always_inline
# endif


#ifndef L4_TO_ADDED_AVOID_WARNINGS
extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);
#endif

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  long int ret;

  __asm__ __volatile__ (
	"xchgl %k0, %1"
	: "=r"(ret), "=m"(*spinlock)
	: "0"(1), "m"(*spinlock)
	: "memory");

  return ret;
}


/* Compare-and-swap for semaphores.  */
# define HAS_COMPARE_AND_SWAP

PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("lock; cmpxchgq %3, %1; sete %0"
			: "=q" (ret), "=m" (*p), "=a" (readval)
			: "r" (newval), "m" (*p), "a" (oldval)
			: "memory");
  return ret;
}

#endif /* !__ASSEMBLER__ */

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS	1

/* Maximum size of the stack if the rlimit is unlimited.  */
#define ARCH_STACK_MAX_SIZE	32*1024*1024

/* The ia32e really want some help to prevent overheating.  */
#define BUSY_WAIT_NOP	__asm__ ("rep; nop")

#endif /* pt-machine.h */
