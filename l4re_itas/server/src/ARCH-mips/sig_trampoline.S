/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

  .text
  .p2align 2
  .globl sigenter
  .type sigenter, @function

sigenter:
  // TODO: save and restore FP state

  // PIC code requires that the called function address is in $25!
  move    $25,$7

  jalr    $25
  nop
  j       sigreturn
  nop
  .size sigenter, . - sigenter


  .globl sigreturn
  .type sigreturn, @function

sigreturn:
  .long 0x04170010 # sigrie 0x10
  .size sigreturn, . - sigreturn

