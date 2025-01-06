/*
 * Copyright (C) 2021, 2024 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above
 */
#pragma once

inline void
switch_stack(unsigned long stack, void (*func)())
{
  register unsigned long a0 asm("a0") = 0;
  asm volatile ( "mv sp, %[stack]   \n\t"
                 "jr %[func]        \n\t"
                 : : [stack] "r" (stack), [func] "r" (func), "r" (a0)
                 : "memory");
}
