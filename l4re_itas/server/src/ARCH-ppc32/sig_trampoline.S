/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

  .text
  .p2align 2

  .globl sigreturn
  .type sigreturn, @function

sigreturn:
  trap
  .size sigreturn, . - sigreturn
