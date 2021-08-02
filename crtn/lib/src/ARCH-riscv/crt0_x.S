/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */

.section .text.init, "ax"
.type _start, @function
.globl _start
_start:

.option push
.option norelax
  la gp, __global_pointer$
.option pop

  lw   a0, sp    /* argc */
#if __riscv_xlen == 32
  addi a1, sp, 4 /* argv */
#else
  addi a1, sp, 8 /* argv */
#endif
  call __main
1:
  j    1b


.section ".data"
  .global __dso_handle
__dso_handle:
  .8byte 0
