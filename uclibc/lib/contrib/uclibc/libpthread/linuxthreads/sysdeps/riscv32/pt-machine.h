/*
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1
#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

#define HAS_COMPARE_AND_SWAP
PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  long int ret, temp;

  __asm__ __volatile__
    ("/* Inline compare & swap */\n"
     "1:\n\t"
     "lr.w      %1,%2\n\t"
     "li        %0,0\n\t"
     "bne       %1,%3,2f\n\t"
     "li        %0,1\n\t"
     "sc.w      %1,%4,%2\n\t"
     "bnez      %1,1b\n"
     "2:\n\t"
     "/* End compare & swap */"
     : "=&r" (ret), "=&r" (temp), "+A" (*p)
     : "r" (oldval), "r" (newval)
     : "memory");

  return ret;
}

extern long int testandset (int *spinlock);

PT_EI long int
testandset (int *spinlock)
{
        unsigned int old = 1;
        int tmp = 1;

        __asm__ __volatile__ (
        "amoswap.w %0, %2, %1"
        : "=r" (old), "+A" (*spinlock)
        : "r" (tmp)
        : "memory");

        return old;
}

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("sp");

#else
#error PT_MACHINE already defined
#endif /* pt-machine.h */
