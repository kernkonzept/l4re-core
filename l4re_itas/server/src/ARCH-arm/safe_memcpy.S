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
  // Copy `size` bytes from `src` (r1) to `dst` (r0) using only general purpose
  // integer registers. The page fault handler diverts faults whose PC lies in
  // the [safe_memcpy, safe_memcpy_end) range to safe_memcpy_fault, which
  // returns false (0). On normal completion safe_memcpy returns true (1).
  //
  // The fault path can only be taken from inside one of the load/store
  // instructions below. At that point r4-r7 and lr have already been saved on
  // the stack, so safe_memcpy_fault can pop them just like the regular epilog.
  .type safe_memcpy, #function
safe_memcpy:
  push    {r4-r7, lr}

  // Bulk copy 16-byte chunks while at least 16 bytes remain.
  cmp     r2, #16
  blo     2f
1:
  ldmia   r1!, {r4-r7}
  stmia   r0!, {r4-r7}
  sub     r2, r2, #16
  cmp     r2, #16
  bhs     1b

2:
  // Drain 4-byte chunks.
  cmp     r2, #4
  blo     4f
3:
  ldr     r3, [r1], #4
  str     r3, [r0], #4
  sub     r2, r2, #4
  cmp     r2, #4
  bhs     3b

4:
  // Drain remaining bytes.
  cmp     r2, #0
  beq     6f
5:
  ldrb    r3, [r1], #1
  strb    r3, [r0], #1
  subs    r2, r2, #1
  bne     5b

6:
  mov     r0, #1
  pop     {r4-r7, pc}
safe_memcpy_end:
  .size safe_memcpy, . - safe_memcpy

  .type safe_memcpy_fault, #function
safe_memcpy_fault:
  mov     r0, #0
  pop     {r4-r7, pc}
  .size safe_memcpy_fault, . - safe_memcpy_fault
