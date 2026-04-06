/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *            Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/* Similar to signal handler: Align stack pointer and skip the red zone. */
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(helper_name) \
  "sub $128, %rsp   \n" \
  "and $~0xf, %rsp  \n" \
  "cld              \n" \
  "call " L4_stringify(helper_name) "\n"
