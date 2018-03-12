/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
  asm volatile ( "mov %[stack], %%rsp   \n\t"
                 "jmp *%[func]          \n\t"
		 : : [stack] "r" (stack), [func] "r" (func), "d" (0)
		 : "memory", "cc");
}

