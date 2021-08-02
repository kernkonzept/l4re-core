/* SPDX-License-Identifier: LGPL-2.1-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
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
