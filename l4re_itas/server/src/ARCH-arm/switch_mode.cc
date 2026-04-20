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

// No virtualization before Armv7
#if __ARM_ARCH >= 7

extern "C" void __l4_syscall_pl0(void);
extern "C" void __l4_syscall_pl1(void);

// By default, ITAS is executing in PL0...
void (*__l4_syscall_indirect)() = &__l4_syscall_pl0;

asm (
  ".global __l4_syscall_pl0\n"
  ".type __l4_syscall_pl0, #function\n"
  "__l4_syscall_pl0:\n"
  "   svc #0\n"
  "   bx lr\n"

  ".arch_extension virt\n"

  ".global __l4_syscall_pl1\n"
  ".type __l4_syscall_pl1, #function\n"
  "__l4_syscall_pl1:\n"
  "   hvc #0\n"
  "   bx lr\n"

  ".global __l4_sys_syscall\n"
  ".type __l4_sys_syscall, #function\n"
  "__l4_sys_syscall:\n"
#ifdef __PIC__
  "   ldr ip, 2f\n"         // load offset to __l4_syscall_indirect
  "1: add ip, pc, ip\n"     // convert to absolute address
  "   ldr pc, [ip]\n"
  "2: .word __l4_syscall_indirect - (1b + 8)\n"
#else
  "   ldr ip, =__l4_syscall_indirect\n"
  "   ldr pc, [ip]\n"
#endif
);

void switch_mode(l4_umword_t ex_regs_flags)
{
  if ((ex_regs_flags & L4_THREAD_EX_REGS_ARM_SET_EL_MASK)
      != L4_THREAD_EX_REGS_ARM_SET_EL_EL1)
    return;

  // The application is running in EL1. Move ourselves in EL1 too so that the
  // executed code of ITAS in application thread context is compatible. Start
  // with the main thread. It is known to wait for IPC and can be safely
  // manipulated.
  L4Re::chksys(L4Re::Env::env()->main_thread()
                 ->ex_regs(~0UL, ~0UL, L4_THREAD_EX_REGS_ARM_SET_EL_EL1),
               "l4re_itas: could not switch to EL1! Wrong kernel?");

  // Now switch ourself, which is the application thread.
  L4::Cap<L4::Thread> self;
  L4Re::chksys(self->ex_regs(~0UL, ~0UL, L4_THREAD_EX_REGS_ARM_SET_EL_EL1));

  // We're running in PL1 now. Quickly patch our syscall instruction!
  __l4_syscall_indirect = &__l4_syscall_pl1;
}

#else // __ARM_ARCH >= 7

void switch_mode(l4_umword_t)
{}

#endif
