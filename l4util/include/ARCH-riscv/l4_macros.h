/*
 * Copyright (C) 2021, 2024 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/util/l4_macros.h>

#ifndef l4_addr_fmt
#if __riscv_xlen == 32
#  define l4_addr_fmt "%08lx"
#else
#  define l4_addr_fmt "%016lx"
#endif
#endif
