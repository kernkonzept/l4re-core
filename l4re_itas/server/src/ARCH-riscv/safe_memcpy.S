/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <sys/asm.h>

  .text

  .globl  safe_memcpy
  .globl  safe_memcpy_fault
  .globl  safe_memcpy_end

  // bool safe_memcpy(void *dst, void const *src, l4_size_t size)
  //
  // Copy `size` bytes from `src` (a1) to `dst` (a0) using only general purpose
  // integer registers. The page fault handler diverts faults whose PC lies in
  // the [safe_memcpy, safe_memcpy_end) range to safe_memcpy_fault, which
  // returns false (0). On normal completion safe_memcpy returns true (1).
  .type   safe_memcpy, @function
safe_memcpy:
#if __riscv_xlen == 64
  // Bulk copy 16-byte chunks (2 doublewords).
  li      t1, 16
  bltu    a2, t1, 2f
1:
  ld      t0, 0(a1)
  ld      t2, 8(a1)
  sd      t0, 0(a0)
  sd      t2, 8(a0)
  addi    a1, a1, 16
  addi    a0, a0, 16
  addi    a2, a2, -16
  bgeu    a2, t1, 1b
2:
  // Drain 8 bytes.
  li      t1, 8
  bltu    a2, t1, 4f
  ld      t0, 0(a1)
  sd      t0, 0(a0)
  addi    a1, a1, 8
  addi    a0, a0, 8
  addi    a2, a2, -8
4:
#else
  // Bulk copy 16-byte chunks (4 words).
  li      t1, 16
  bltu    a2, t1, 4f
1:
  lw      t0, 0(a1)
  lw      t2, 4(a1)
  lw      t3, 8(a1)
  lw      t4, 12(a1)
  sw      t0, 0(a0)
  sw      t2, 4(a0)
  sw      t3, 8(a0)
  sw      t4, 12(a0)
  addi    a1, a1, 16
  addi    a0, a0, 16
  addi    a2, a2, -16
  bgeu    a2, t1, 1b
4:
#endif
  // Drain 4-byte chunks.
  li      t1, 4
  bltu    a2, t1, 6f
5:
  lw      t0, 0(a1)
  sw      t0, 0(a0)
  addi    a1, a1, 4
  addi    a0, a0, 4
  addi    a2, a2, -4
  bgeu    a2, t1, 5b
6:
  // Drain remaining bytes.
  beqz    a2, 8f
7:
  lb      t0, 0(a1)
  sb      t0, 0(a0)
  addi    a1, a1, 1
  addi    a0, a0, 1
  addi    a2, a2, -1
  bnez    a2, 7b
8:
  li      a0, 1
  ret
safe_memcpy_end:
  .size   safe_memcpy, . - safe_memcpy

  .type   safe_memcpy_fault, @function
safe_memcpy_fault:
  li      a0, 0
  ret
  .size   safe_memcpy_fault, . - safe_memcpy_fault
