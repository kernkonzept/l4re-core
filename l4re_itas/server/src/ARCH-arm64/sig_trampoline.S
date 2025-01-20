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
  stp   q0, q1,   [x19, #16 *  0]
  stp   q2, q3,   [x19, #16 *  2]
  stp   q4, q5,   [x19, #16 *  4]
  stp   q6, q7,   [x19, #16 *  6]
  stp   q8, q9,   [x19, #16 *  8]
  stp   q10, q11, [x19, #16 * 10]
  stp   q12, q13, [x19, #16 * 12]
  stp   q14, q15, [x19, #16 * 14]
  stp   q16, q17, [x19, #16 * 16]
  stp   q18, q19, [x19, #16 * 18]
  stp   q20, q21, [x19, #16 * 20]
  stp   q22, q23, [x19, #16 * 22]
  stp   q24, q25, [x19, #16 * 24]
  stp   q26, q27, [x19, #16 * 26]
  stp   q28, q29, [x19, #16 * 28]
  stp   q30, q31, [x19, #16 * 30]

  mrs   x9, fpsr
  str   x9, [x19, #16 * 32 + 0]
  mrs   x9, fpcr
  str   x9, [x19, #16 * 32 + 8]

  blr   x16

  ldr   x9, [x19, #16 * 32 + 8]
  msr   fpcr, x9
  ldr   x9, [x19, #16 * 32 + 0]
  msr   fpsr, x9

  ldp   q0, q1,   [x19, #16 *  0]
  ldp   q2, q3,   [x19, #16 *  2]
  ldp   q4, q5,   [x19, #16 *  4]
  ldp   q6, q7,   [x19, #16 *  6]
  ldp   q8, q9,   [x19, #16 *  8]
  ldp   q10, q11, [x19, #16 * 10]
  ldp   q12, q13, [x19, #16 * 12]
  ldp   q14, q15, [x19, #16 * 14]
  ldp   q16, q17, [x19, #16 * 16]
  ldp   q18, q19, [x19, #16 * 18]
  ldp   q20, q21, [x19, #16 * 20]
  ldp   q22, q23, [x19, #16 * 22]
  ldp   q24, q25, [x19, #16 * 24]
  ldp   q26, q27, [x19, #16 * 26]
  ldp   q28, q29, [x19, #16 * 28]
  ldp   q30, q31, [x19, #16 * 30]

  b sigreturn
  .size sigenter, . - sigenter


  .globl sigreturn
  .type sigreturn, #function

sigreturn:
  brk #123
  .size sigreturn, . - sigreturn
