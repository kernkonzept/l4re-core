/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
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

