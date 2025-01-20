/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

  .text
  .p2align 4
  .globl sigenter
  .type sigenter, @function

# Arguments:
#   esp, esp+4 and esp+8: signal handler arguments, esp+12: empty
#   edi: pointer to the FPU-state save area (callee saved)
#   esi: signal handler address
#
# ATTENTION: we are "called" with the wrong stack alignment! Per x86 calling
# convention, %esp & 0x0f == 12 should hold on entry.

sigenter:
  cld
  sub     $4, %esp
  fxsave  (%edi)
  push    12(%esp)
  push    12(%esp)
  push    12(%esp)
  call    *%esi
  fxrstor (%edi)

  # This may look strange but we have to unwind the whole sigenter frame. The
  # generic code in signals.cc will assume that the stack pointer is at
  # ucontext_t*. This means we have to remove 32 bytes: 16 bytes of sigenter
  # arguments, 4 bytes local stack frame and 12 bytes of sighandler arguments.
  add     $(16+4+12), %esp
  jmp     sigreturn

  .size sigenter, . - sigenter


  .globl sigreturn
  .type sigreturn, @function

sigreturn:
  ud2
  .size sigreturn, . - sigreturn
