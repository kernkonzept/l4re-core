/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License 2.  See the file "COPYING-GPL-2" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 2013 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 */
/**
 * \file
 * MIPS-specific vCPU interface.
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
  L4_VCPU_STATE_VERSION = 0x12,

  L4_VCPU_STATE_SIZE = 0x200,
  L4_VCPU_STATE_EXT_SIZE = L4_PAGESIZE,
};

/**
 * Offsets for vCPU state layouts
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_state_offset
{
  L4_VCPU_OFFSET_EXT_STATE = 0x400, ///< Offset where extended state begins
  L4_VCPU_OFFSET_EXT_INFOS = 0x200, ///< Offset where extended infos begin
};

/**
 * \brief vCPU registers.
 * \ingroup l4_vcpu_api
 *
 * l4_exc_regs_t matches l4_vcpu_regs_t and corresponds to
 * fiasco/src/kern/mips32/trap_state.cpp: Trap_state_regs and
 * entry_frame-mips32.cpp: Syscall_frame and Return_frame
 */
typedef l4_exc_regs_t l4_vcpu_regs_t;

/**
 * Architecture-specific vCPU state.
 */
typedef struct l4_vcpu_arch_state_t
{
  l4_umword_t ulr;
} l4_vcpu_arch_state_t;

/**
 * \brief vCPU message registers.
 * \ingroup l4_vcpu_api
 *
 * l4_vcpu_ipc_regs_t register usage matches the implementation of l4_ipc() in
 * l4sys/include/ARCH-mips/L4API-l4f/ipc.h
 */
typedef struct l4_vcpu_ipc_regs_t
{
  void *utcb;           /* s0 */
  l4_umword_t dest;     /* s1 */
  l4_umword_t timeout;  /* s2 */
  l4_msgtag_t tag;      /* s3 */
  l4_umword_t label;    /* s4 */
} l4_vcpu_ipc_regs_t;
