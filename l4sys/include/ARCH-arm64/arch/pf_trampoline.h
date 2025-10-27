/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Page fault trampoline handler definitions for ARM64.
 */

#pragma once

/**
 * \defgroup l4_pf_trampoline_api_arm64 ARM64 page fault trampoline handler.
 * \ingroup  l4_pf_trampoline_api
 */

/**
 * Register state for page fault trampoline handlers.
 * \ingroup l4_pf_trampoline_api_arm64
 */
struct l4_pf_trampoline_regs_t
{
  l4_umword_t reserved1[34]; // eret_work, GP registers, err
  l4_umword_t pfa;           /**< page fault address */
  l4_umword_t reserved2[1];  // sp
  l4_umword_t ip;            /**< instruction pointer of faulting instruction */
  l4_umword_t reserved3[1];  // pstate
};
