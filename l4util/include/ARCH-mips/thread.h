/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/compiler.h>

#if _MIPS_SZPTR == 32
#define L4UTIL_THREAD_START_LOAD_FUNC_ADDR(func) "la $t9," func "\n"
#else
#define L4UTIL_THREAD_START_LOAD_FUNC_ADDR(func) "dla $t9," func "\n"
#endif

#if _MIPS_SIM == _ABIO32
#define L4UTIL_THREAD_START_SETUP_GP  ".cpload $25;"
#else
#define L4UTIL_THREAD_START_SETUP_GP \
" bal 10f \n" \
"  nop    \n" \
"10:      \n" \
" .cpsetup $31, $25, 10b \n"
#endif


#define __L4UTIL_THREAD_FUNC(name) \
EXTERN_C_BEGIN \
static void  __attribute__((used)) name##_worker_function(void); \
asm ( \
  ".type " #name ", function \n" \
  ".global " #name " \n" \
  #name ": \n .set push; .set noreorder;" \
  L4UTIL_THREAD_START_SETUP_GP \
  L4UTIL_THREAD_START_LOAD_FUNC_ADDR(#name "_worker_function") \
  "  jal $t9 \n" \
  "   nop    \n"\
  ".set pop" \
); \
EXTERN_C_END \
static L4_NORETURN void name##_worker_function(void)

#define L4UTIL_THREAD_FUNC(name) __L4UTIL_THREAD_FUNC(name)

#define __L4UTIL_THREAD_STATIC_FUNC(name) \
EXTERN_C_BEGIN \
void __attribute__((visibility("internal"))) name(void); \
static void __attribute__((used)) name ##_worker_function(void); \
asm ( \
  ".type " #name ", function \n" \
  #name ": \n .set push; .set noreorder;" \
  L4UTIL_THREAD_START_SETUP_GP \
  L4UTIL_THREAD_START_LOAD_FUNC_ADDR(#name "_worker_function") \
  "  jal $t9 \n" \
  "   nop    \n"\
  ".set pop" \
); \
EXTERN_C_END \
static L4_NORETURN void name##_worker_function(void)

#define L4UTIL_THREAD_STATIC_FUNC(name) __L4UTIL_THREAD_STATIC_FUNC(name)


#include_next <l4/util/thread.h>
