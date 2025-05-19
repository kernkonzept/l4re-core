/*
 * Copyright (C) 2016-2017, 2023-2024 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>

#if _MIPS_SZPTR == 32
# define L4UTIL_THREAD_START_LOAD_FUNC_ADDR(func) "  la $t9," func "\n"
#else
# define L4UTIL_THREAD_START_LOAD_FUNC_ADDR(func) "  dla $t9," func "\n"
#endif

#if _MIPS_SIM == _ABIO32
# define L4UTIL_THREAD_START_SETUP_GP \
"  bal 10f \n" \
"   nop    \n" \
"10:       \n" \
"  .cpload $31 \n"
#else
# define L4UTIL_THREAD_START_SETUP_GP \
" bal 10f \n" \
"  nop    \n" \
"10:      \n" \
"  .cpsetup $31, $25, 10b \n"
#endif

#define L4UTIL_THREAD_FUNC_MIPS_TEMPLATE(name, locality) \
L4_BEGIN_DECLS \
locality L4_NORETURN void name(void) \
{ \
  asm("  .set push \n" \
      "  .set noreorder \n" \
      L4UTIL_THREAD_START_SETUP_GP \
      L4UTIL_THREAD_START_LOAD_FUNC_ADDR(#name "_mips_helper_func") \
      "  jal $t9 \n" \
      "   nop    \n"\
      "  .set pop" \
  ); \
  __builtin_unreachable(); \
} \
static L4_NORETURN void __attribute__((used)) name ##_mips_helper_func(void); \
L4_END_DECLS \
static L4_NORETURN void name##_mips_helper_func(void)

#define L4UTIL_THREAD_STATIC_FUNC(name) \
        L4UTIL_THREAD_FUNC_MIPS_TEMPLATE(name, static)

#define L4UTIL_THREAD_FUNC(name) \
        L4UTIL_THREAD_FUNC_MIPS_TEMPLATE(name, )

#include_next <l4/util/thread.h>
