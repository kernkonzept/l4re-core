/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
.section .text.init, "ax"
.type _start, @function
.globl _start
_start:
  lis  %r1, crt0_stack_high@ha  /* load stack pointer */
  addi %r1, %r1, crt0_stack_high@l
  b    init

.section ".bss", "aw"

.balign 16
.global crt0_stack_low
crt0_stack_low:
  .space 4096
.global crt0_stack_high
crt0_stack_high:
