/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

  .text
  .p2align 2
  .globl sigenter
  .type sigenter, #function

sigenter:
  // TODO: save and restore FP state
  blx r12
  b sigreturn
  .size sigenter, . - sigenter


  .globl sigreturn
  .type sigreturn, #function

sigreturn:
  .long 0xe1600070 // smc
  .size sigreturn, . - sigreturn
