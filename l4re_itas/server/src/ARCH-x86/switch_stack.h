/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

inline void
switch_stack(unsigned long stack, void (*func)())
{
  asm volatile ( "mov %[stack], %%esp   \n\t"
		 "xor %%ebp, %%ebp      \n\t"
                 "jmp *%[func]          \n\t"
		 : : [stack] "r" (stack), [func] "r" (func), "d" (0)
		 : "memory", "cc");
}

