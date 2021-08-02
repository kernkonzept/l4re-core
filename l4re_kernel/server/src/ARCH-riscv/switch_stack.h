/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
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
