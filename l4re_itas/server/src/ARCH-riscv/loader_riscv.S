/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <sys/asm.h>

ENTRY(loader_thread)
.option push
.option norelax
  la   gp, __global_pointer$
.option pop
  call loader_thread_c
END(loader_thread)
