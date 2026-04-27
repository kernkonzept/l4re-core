/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
  .text

  .global safe_memcpy
  .global safe_memcpy_fault
  .global safe_memcpy_end

  // bool safe_memcpy(void *dst, void const *src, l4_size_t size)
  //
  // Copy `size` bytes from `src` (x1) to `dst` (x0). Uses only general-purpose
  // integer registers so it can be called from contexts where the FPU is not
  // available. The page fault handler recognizes faults whose PC lies in the
  // [safe_memcpy, safe_memcpy_end) range and resumes execution with w0 = 0.
  // On normal completion w0 is set to 1.
  .type safe_memcpy, #function
safe_memcpy:
  // Bulk copy in 16-byte units while at least 16 bytes remain.
  cmp   x2, #16
  b.lo  2f
1:
  ldp   x3, x4, [x1], #16
  stp   x3, x4, [x0], #16
  sub   x2, x2, #16
  cmp   x2, #16
  b.hs  1b

  // Optionally drain another 8 bytes.
2:
  tbz   x2, #3, 3f
  ldr   x3, [x1], #8
  str   x3, [x0], #8

  // Optionally drain 4, 2 and 1 byte tails.
3:
  tbz   x2, #2, 4f
  ldr   w3, [x1], #4
  str   w3, [x0], #4
4:
  tbz   x2, #1, 5f
  ldrh  w3, [x1], #2
  strh  w3, [x0], #2
5:
  tbz   x2, #0, 6f
  ldrb  w3, [x1]
  strb  w3, [x0]
6:
  mov   w0, #1
  ret
safe_memcpy_end:
  .size safe_memcpy, . - safe_memcpy

  .type safe_memcpy_fault, #function
safe_memcpy_fault:
  mov   w0, #0
  ret
  .size safe_memcpy_fault, . - safe_memcpy_fault
