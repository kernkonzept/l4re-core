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

/* BID_VARIANT_FLAG_NOFPU: Code runs in contexts of application threads! */

sigenter:
  // TODO: save FP regs
  jalr  a3
  tail  sigreturn

  .size sigenter, . - sigenter


  .globl sigreturn
  .type sigreturn, @function

sigreturn:
  unimp

  .size sigreturn, . - sigreturn
