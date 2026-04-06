/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *            Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#define L4UTIL_THREAD_CXX_FUNC_HELPER_PROTO_ATTR \
  __attribute__((regparm(3)))

#define L4UTIL_THREAD_CXX_FUNC_PROTO(fn_name, ...) \
  [[noreturn]] static void fn_name(__VA_ARGS__) \
  asm (L4_stringify(_L4UTIL_THREAD_STUB_NAME(fn_name))) \
  L4UTIL_THREAD_CXX_FUNC_HELPER_PROTO_ATTR

#define L4UTIL_THREAD_CXX_FUNC_INTERRUPT_HELPER_PROTO_ATTR \
  L4UTIL_THREAD_CXX_FUNC_HELPER_PROTO_ATTR

/* Similar to signal handler: Align stack pointer. */
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(helper_name) \
  "and $~0xf, %esp  \n" \
  "cld              \n" \
  "call " L4_stringify(helper_name) "\n"
