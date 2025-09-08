/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */

#include <sys/asm.h>

.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
  REG_L t0, 0(sp) /* entry point */
  REG_L ra, (1*SZREG)(sp) /* return address */
  REG_L a0, (2*SZREG)(sp) /* argument */

  /* align stack */
  andi sp, sp, ALMASK

.option push
.option norelax
  /* load global pointer */
  la   gp, __global_pointer$
.option pop

  jr t0
