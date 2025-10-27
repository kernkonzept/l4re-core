/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Page fault trampoline handler definitions for ARM.
 */

#pragma once

/**
 * \defgroup l4_pf_trampoline_api_arm ARM page fault trampoline handler.
 * \ingroup  l4_pf_trampoline_api
 */

/**
 * Register state for page fault trampoline handlers.
 * \ingroup l4_pf_trampoline_api_arm
 */
struct l4_pf_trampoline_regs_t
{
  l4_umword_t pfa;           /**< page fault address */
  l4_umword_t reserved1[1];  // err
  l4_umword_t reserved2[16]; // GP registers, sp, ulr, dummy
  l4_umword_t ip;            /**< instruction pointer of faulting instruction */
  l4_umword_t reserved3[1];  // cpsr
};
