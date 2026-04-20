/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/sys/thread>

#include "switch_mode.h"

extern "C" void __l4_syscall_el0(void);
extern "C" void __l4_syscall_el1(void);

// By default, ITAS is executing in EL0...
void (*__l4_syscall_indirect)() = &__l4_syscall_el0;

asm (
  ".global __l4_syscall_el0\n"
  ".type __l4_syscall_el0, @function\n"
  "__l4_syscall_el0:\n"
  "   svc #0\n"
  "   ret\n"

  ".global __l4_syscall_el1\n"
  ".type __l4_syscall_el1, @function\n"
  "__l4_syscall_el1:\n"
  "   hvc #0\n"
  "   ret\n"

  ".global __l4_sys_syscall\n"
  ".type __l4_sys_syscall, @function\n"
  "__l4_sys_syscall:\n"
  "   ldr x16, __l4_syscall_indirect\n"
  "   br  x16\n"
);

void switch_mode(l4_umword_t ex_regs_flags)
{
  if ((ex_regs_flags & L4_THREAD_EX_REGS_ARM64_SET_EL_MASK)
      != L4_THREAD_EX_REGS_ARM64_SET_EL_EL1)
    return;

  // The application is running in EL1. Move ourselves in EL1 too so that the
  // executed code of ITAS in application thread context is compatible. Start
  // with the main thread. It is known to wait for IPC and can be safely
  // manipulated.
  L4Re::chksys(L4Re::Env::env()->main_thread()
                 ->ex_regs(~0UL, ~0UL, L4_THREAD_EX_REGS_ARM64_SET_EL_EL1),
               "l4re_itas: could not switch to EL1! Wrong kernel?");

  // Now switch ourself, which is the application thread.
  L4::Cap<L4::Thread> self;
  L4Re::chksys(self->ex_regs(~0UL, ~0UL, L4_THREAD_EX_REGS_ARM64_SET_EL_EL1));

  // We're running in EL1 now. Quickly patch our syscall instruction!
  __l4_syscall_indirect = &__l4_syscall_el1;
}
