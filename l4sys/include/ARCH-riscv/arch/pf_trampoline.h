/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Page fault trampoline handler definitions for RISC-V.
 */

#pragma once

/**
 * \defgroup l4_pf_trampoline_api_riscv RISC-V page fault trampoline handler.
 * \ingroup  l4_pf_trampoline_api
 */

/**
 * Register state for page fault trampoline handlers.
 * \ingroup l4_pf_trampoline_api_riscv
 */
struct l4_pf_trampoline_regs_t
{
  l4_umword_t reserved1[1];  // eret_work
  l4_umword_t reserved2[31]; // GP registers
  l4_umword_t ip;            /**< instruction pointer of faulting instruction */
  l4_umword_t reserved3[2];  // status, cause
  l4_umword_t pfa;           /**< page fault address */
  l4_umword_t reserved4[1];  // hstatus
};
