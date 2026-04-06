/*
 * Copyright (C) 2021, 2024-2025 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#define L4UTIL_THREAD_CXX_FUNC_IMPL_STUB(helper_name) \
  ".option push      \n" \
  ".option norelax   \n" \
  "  la   gp, __global_pointer$ \n" \
  ".option pop       \n" \
  "jal " L4_stringify(helper_name) " \n"

/* Similar to signal handler: Align stack pointer. */
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(helper_name) \
  ".option push      \n" \
  ".option norelax   \n" \
  "  la   gp, __global_pointer$ \n" \
  ".option pop       \n" \
  "andi sp, sp, ~0xf \n" \
  "jal " L4_stringify(helper_name) " \n"

#define L4UTIL_THREAD_FUNC_RISCV_TEMPLATE(name, locality) \
L4_BEGIN_DECLS \
locality L4_NORETURN void name(void) \
{ \
  asm ( \
  L4UTIL_THREAD_CXX_FUNC_IMPL_STUB(name ## _helper) \
  ); \
  __builtin_trap(); \
} \
static L4_NORETURN void __attribute__((used)) \
        name ## _helper(void); \
L4_END_DECLS \
static L4_NORETURN void name ## _helper(void)

#define L4UTIL_THREAD_STATIC_FUNC(name) \
        L4UTIL_THREAD_FUNC_RISCV_TEMPLATE(name, static)

#define L4UTIL_THREAD_FUNC(name) \
        L4UTIL_THREAD_FUNC_RISCV_TEMPLATE(name, )
