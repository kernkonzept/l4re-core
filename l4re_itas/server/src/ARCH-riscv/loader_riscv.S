/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#include <sys/asm.h>

ENTRY(loader_thread)
.option push
.option norelax
  la   gp, __global_pointer$
.option pop
  call loader_thread_c
END(loader_thread)
