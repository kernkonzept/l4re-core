/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
.section .text.init, "ax"
.type _start, @function
.globl _start
_start:
  lis  %r1, crt0_stack_high@ha  /* load stack pointer */
  addi %r1, %r1, crt0_stack_high@l
  b    init

.section ".bss", "aw"

.global crt0_stack_low
crt0_stack_low:
  .space 4096
.global crt0_stack_high
crt0_stack_high:
