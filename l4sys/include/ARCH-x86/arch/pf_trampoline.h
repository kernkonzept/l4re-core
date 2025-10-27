/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Page fault trampoline handler definitions for x86.
 */

#pragma once

/**
 * \defgroup l4_pf_trampoline_api_x86 x86 page fault trampoline handler.
 * \ingroup  l4_pf_trampoline_api
 */

/**
 * Register state for page fault trampoline handlers.
 * \ingroup l4_pf_trampoline_api_x86
 */
struct l4_pf_trampoline_regs_t
{
  l4_umword_t reserved1[4]; // es, ds, gs, fs
  l4_umword_t reserved2[3]; // edi, esi, ebp
  l4_umword_t pfa;          /**< page fault address */
  l4_umword_t reserved3[4]; // ebx, edx, ecx, eax
  l4_umword_t reserved4[2]; // trapno, err
  l4_umword_t ip;           /**< instruction pointer of faulting instruction */
  l4_umword_t reserved5[4]; // cs, eflags, sp, ss
};
