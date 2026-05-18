/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#define L4UTIL_THREAD_CXX_FUNC_PROTO(fn_name, ...) \
  [[noreturn]] static void fn_name(__VA_ARGS__) \
  asm (L4_stringify(fn_name ## _stub)) \
  __attribute__((target("arm"))); \
  [[noreturn]] static void fn_name ## _from_asm(__VA_ARGS__) \
  asm (L4_stringify(fn_name ## _from_asm))

/* Similar to signal handler: Align stack pointer. */
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(from_asm_name) \
  ".arm           \n" \
  "bic sp, sp, #7 \n" \
  "bl " L4_stringify(from_asm_name) "\n"
