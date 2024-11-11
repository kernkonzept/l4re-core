/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

inline void
switch_stack(unsigned long stack, void (*func)())
{
//  register unsigned long r3 asm("r3") = 0;
  asm volatile ( "mr    %%r1, %[stack] \n\t"
                 "mtctr %[func]        \n\t"
                 "bctr                 \n\t"
		 : : [stack] "r" (stack), [func] "r" (func) //, "r" (r3)
		 : "memory", "ctr");
}

