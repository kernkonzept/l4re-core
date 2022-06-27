/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2022, 2021 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 */

#include <cstdio>
#include <l4/util/util.h>

struct Exception_el1_registers
{
  // Regs x19-x29 are not part of the structure since they are callee saved.
  l4_uint64_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x10;
  l4_uint64_t x11, x12, x13, x14, x15, x16, x17, x18, x30;
};

extern "C" void
__l4_arm64_exception_handler(l4_umword_t, Exception_el1_registers *,
                             l4_umword_t, l4_umword_t, l4_umword_t spsr);
void
__l4_arm64_exception_handler(l4_umword_t vector, Exception_el1_registers *,
                             l4_umword_t /*esr*/, l4_umword_t elr,
                             l4_umword_t spsr)
{
  fprintf(stderr, "Exception triggered vector 0x%lx, elr 0x%lx, spsr 0x%08lx\n",
          vector, elr, spsr);
  l4_sleep_forever();
}
