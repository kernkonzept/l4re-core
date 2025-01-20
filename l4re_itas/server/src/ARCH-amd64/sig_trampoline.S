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
#   %rdi, %rsi and %rdx: signal handler arguments
#   %rcx: pointer to the FPU-state save area
#   %r8: signal handler address
#
# ATTENTION: we are "called" with the wrong stack alignment! Per amd64 calling
# convention, %rsp & 0x0f == 8 should hold on entry.

sigenter:
  sub       $8, %rsp
  cld
  fxsave64  (%rcx)
  push      %rcx
  call      *%r8
  pop       %rcx
  fxrstor64 (%rcx)
  add       $8, %rsp
  jmp sigreturn

  .size sigenter, . - sigenter


  .globl sigreturn
  .type sigreturn, @function

sigreturn:
  ud2
  .size sigreturn, . - sigreturn
