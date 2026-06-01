/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/* Similar to signal handler: Align stack pointer. */
#define L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB(from_asm_name) \
  "ta 0x1"
