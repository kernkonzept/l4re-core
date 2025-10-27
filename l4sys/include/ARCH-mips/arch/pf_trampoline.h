/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Page fault trampoline handler definitions for MIPS.
 */

#pragma once

/**
 * \defgroup l4_pf_trampoline_api_mips MIPS page fault trampoline handler.
 * \ingroup  l4_pf_trampoline_api
 */

/**
 * Register state for page fault trampoline handlers.
 * \ingroup l4_pf_trampoline_api_mips
 */
struct l4_pf_trampoline_regs_t
{
  l4_umword_t reserved1[2];  // bad_instr_p, bad_instr
  l4_umword_t reserved2[32]; // GP registers
  l4_umword_t reserved3[2];  // hi, lo
  l4_umword_t pfa;           /**< page fault address */
  l4_umword_t reserved4[2];  // cause, status
  l4_umword_t ip;            /**< instruction pointer of faulting instruction */
};
