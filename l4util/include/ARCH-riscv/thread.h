/*
 * Copyright (C) 2021, 2024 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>

#define L4UTIL_THREAD_FUNC_RISCV_TEMPLATE(name, locality) \
L4_BEGIN_DECLS \
locality L4_NORETURN void name(void) \
{ \
  asm( \
    ".option push \n" \
    ".option norelax \n" \
    "  la   gp, __global_pointer$ \n" \
    ".option pop \n" \
    "jal " #name "_riscv_helper_func \n" \
  ); \
  __builtin_unreachable(); \
} \
static L4_NORETURN void __attribute__((used)) name ##_riscv_helper_func(void); \
L4_END_DECLS \
static L4_NORETURN void name##_riscv_helper_func(void)

#define L4UTIL_THREAD_STATIC_FUNC(name) \
        L4UTIL_THREAD_FUNC_RISCV_TEMPLATE(name, static)

#define L4UTIL_THREAD_FUNC(name) \
        L4UTIL_THREAD_FUNC_RISCV_TEMPLATE(name, )

#include_next <l4/util/thread.h>
