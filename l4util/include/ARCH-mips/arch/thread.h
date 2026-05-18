/*
 * Copyright (C) 2016-2017, 2023-2025 Kernkonzept GmbH.
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

# define L4UTIL_THREAD_MIPS_STACK_RESERVE " add $sp, $sp, -32 \n"

#else

# define L4UTIL_THREAD_START_SETUP_GP \
" bal 10f \n" \
"  nop    \n" \
"10:      \n" \
"  .cpsetup $31, $25, 10b \n"

# define L4UTIL_THREAD_MIPS_STACK_RESERVE

#endif

#define L4UTIL_THREAD_CXX_FUNC_IMPL_STUB(from_asm_name) \
  "  .set push          \n" \
  "  .set noreorder     \n" \
  L4UTIL_THREAD_START_SETUP_GP \
  L4UTIL_THREAD_START_LOAD_FUNC_ADDR(L4_stringify(from_asm_name)) \
  "  jal $t9            \n" \
  "   nop               \n" \
  "  .set pop"

/* Similar to signal handler: Align stack pointer and (for MIPS-32), skip
 * reserved stack area. */
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(from_asm_name) \
  "  .set push          \n" \
  "  .set noreorder     \n" \
  L4UTIL_THREAD_START_SETUP_GP \
  L4UTIL_THREAD_START_LOAD_FUNC_ADDR(L4_stringify(from_asm_name)) \
  "  li $s0, ~0xf       \n" \
  L4UTIL_THREAD_MIPS_STACK_RESERVE \
  "  and $sp, $sp, $s0  \n" \
  "  jal $t9            \n" \
  "   nop               \n" \
  "  .set pop"

#define L4UTIL_THREAD_FUNC_MIPS_TEMPLATE(name, locality) \
L4_BEGIN_DECLS \
locality L4_NORETURN void name(void) \
{ \
  asm ( \
  L4UTIL_THREAD_CXX_FUNC_IMPL_STUB(name ## _from_asm) \
  ); \
  __builtin_trap(); \
} \
static L4_NORETURN void __attribute__((used)) \
        name ## _from_asm(void); \
L4_END_DECLS \
static L4_NORETURN void name ## _from_asm(void)

#define L4UTIL_THREAD_STATIC_FUNC(name) \
        L4UTIL_THREAD_FUNC_MIPS_TEMPLATE(name, static)

#define L4UTIL_THREAD_FUNC(name) \
        L4UTIL_THREAD_FUNC_MIPS_TEMPLATE(name, )
