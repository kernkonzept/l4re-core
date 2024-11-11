/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

.section .text.init, "ax"
.type _start, @function
.globl _start
_start:

.option push
.option norelax
  la gp, __global_pointer$
.option pop

  la sp, crt0_stack_high
  la ra, _exit
  j init

.section ".bss", "aw"
.balign 16
.global crt0_stack_low
crt0_stack_low:
  .space 4096
.global crt0_stack_high
crt0_stack_high:
