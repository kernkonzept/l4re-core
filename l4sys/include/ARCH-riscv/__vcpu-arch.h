/* SPDX-License-Identifier: ((GPL-2.0-only WITH mif-exception) OR LicenseRef-kk-custom) */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
/**
 * \file
 * RISC-V-specific vCPU interface.
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

enum
{
  /**
   * Architecture-specific version ID.
   *
   * This ID must match the version field in the l4_vcpu_state_t structure
   * after enabling vCPU mode or extended vCPU mode for a thread.
   */
  L4_VCPU_STATE_VERSION = 0x0
};

/**
 * \brief vCPU registers.
 * \ingroup l4_vcpu_api
 */
typedef l4_exc_regs_t l4_vcpu_regs_t;

/**
 * Architecture-specific vCPU state.
 */
typedef struct l4_vcpu_arch_state_t
{
  l4_umword_t host_tp;
  l4_umword_t host_gp;
} l4_vcpu_arch_state_t;

/**
 * \brief vCPU message registers.
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_ipc_regs_t
{
  l4_msgtag_t tag;      /* a0 */
  l4_umword_t dest;     /* a1 */
  l4_umword_t timeout;  /* a2 */
  l4_umword_t label;    /* a3 */
} l4_vcpu_ipc_regs_t;
