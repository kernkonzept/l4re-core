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
  // Copy `size` bytes from `src` to `dst` using only general purpose integer
  // registers. The page fault handler diverts faults whose PC lies in the
  // [safe_memcpy, safe_memcpy_end) range to safe_memcpy_fault, which restores
  // the callee-saved registers pushed at the start of safe_memcpy and returns
  // false (0). On normal completion safe_memcpy returns true (1).
  .type safe_memcpy, @function
safe_memcpy:
  pushl   %edi
  pushl   %esi
  movl    12(%esp), %edi    // dst
  movl    16(%esp), %esi    // src
  movl    20(%esp), %ecx    // size
  cld
  rep movsb
  movl    $1, %eax
  popl    %esi
  popl    %edi
  ret
safe_memcpy_end:
  .size safe_memcpy, . - safe_memcpy

  // The fault path can only be taken from inside `rep movsb`, at which point
  // %esi and %edi are still saved on the stack from the prologue.
  .type safe_memcpy_fault, @function
safe_memcpy_fault:
  popl    %esi
  popl    %edi
  xorl    %eax, %eax
  ret
  .size safe_memcpy_fault, . - safe_memcpy_fault
