/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *            Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

// Similar to signal handler: Align stack pointer.
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(helper_name) \
  "mov x16, sp        \n" \
  "and sp, x16, #~0xf \n" \
  "bl " L4_stringify(helper_name) "\n"
