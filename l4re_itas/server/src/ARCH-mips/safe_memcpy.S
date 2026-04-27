/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <sys/regdef.h>
#include <sys/asm.h>

  .text
  .set    reorder

  .globl  safe_memcpy
  .globl  safe_memcpy_fault
  .globl  safe_memcpy_end

  // bool safe_memcpy(void *dst, void const *src, l4_size_t size)
  //
  // Copy `size` bytes from `src` ($a1) to `dst` ($a0) using only general
  // purpose integer registers. The page fault handler diverts faults whose
  // PC lies in the [safe_memcpy, safe_memcpy_end) range to safe_memcpy_fault,
  // which returns false (0). On normal completion safe_memcpy returns true (1).
  .ent    safe_memcpy
  .type   safe_memcpy, @function
safe_memcpy:
  beqz    a2, 2f
1:
  lb      t0, 0(a1)
  PTR_ADDIU a1, a1, 1
  sb      t0, 0(a0)
  PTR_ADDIU a0, a0, 1
  PTR_ADDIU a2, a2, -1
  bnez    a2, 1b
2:
  li      v0, 1
  jr      ra
  .end    safe_memcpy
safe_memcpy_end:
  .size   safe_memcpy, . - safe_memcpy

  .ent    safe_memcpy_fault
  .type   safe_memcpy_fault, @function
safe_memcpy_fault:
  move    v0, zero
  jr      ra
  .end    safe_memcpy_fault
  .size   safe_memcpy_fault, . - safe_memcpy_fault
