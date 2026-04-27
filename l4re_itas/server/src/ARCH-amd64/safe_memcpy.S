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
  // Copy `size` bytes from `src` (%rsi) to `dst` (%rdi). Uses only general
  // purpose integer registers. The page fault handler diverts faults whose
  // PC lies in the [safe_memcpy, safe_memcpy_end) range to safe_memcpy_fault,
  // which returns false (0). On normal completion safe_memcpy returns true (1).
  .type safe_memcpy, @function
safe_memcpy:
  movq    %rdx, %rcx
  cld
  rep movsb
  movl    $1, %eax
  ret
safe_memcpy_end:
  .size safe_memcpy, . - safe_memcpy

  .type safe_memcpy_fault, @function
safe_memcpy_fault:
  xorl    %eax, %eax
  ret
  .size safe_memcpy_fault, . - safe_memcpy_fault
