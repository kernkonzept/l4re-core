/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/**
 * \file
 * x86-specific vCPU interface.
 */
#pragma once

#include <l4/sys/types.h>

enum
{
  /**
   * Architecture-specific version ID.
   *
   * This ID must match the version field in the l4_vcpu_state_t structure
   * after enabling vCPU mode or extended vCPU mode for a thread.
   */
  L4_VCPU_STATE_VERSION = 0x46,

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
 */
typedef struct l4_vcpu_regs_t
{
  l4_umword_t es;     /**< es register */
  l4_umword_t ds;     /**< ds register */
  l4_umword_t gs;     /**< gs register */
  l4_umword_t fs;     /**< fs register */

  l4_umword_t di;     /**< edi register */
  l4_umword_t si;     /**< esi register */
  l4_umword_t bp;     /**< ebp register */
  l4_umword_t pfa;    /**< page fault address */
  l4_umword_t bx;     /**< ebx register */
  l4_umword_t dx;     /**< edx register */
  l4_umword_t cx;     /**< ecx register */
  l4_umword_t ax;     /**< eax register */

  l4_umword_t trapno; /**< trap number */
  l4_umword_t err;    /**< error code */

  l4_umword_t ip;     /**< instruction pointer */
  l4_umword_t dummy1; /**< dummy \internal */
  l4_umword_t flags;  /**< eflags */
  l4_umword_t sp;     /**< stack pointer */
  l4_umword_t ss;     /**< ss register */
} l4_vcpu_regs_t;

/**
 * Architecture-specific vCPU state.
 */
typedef struct l4_vcpu_arch_state_t {} l4_vcpu_arch_state_t;

/**
 * \brief vCPU message registers.
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_ipc_regs_t
{
  l4_umword_t _res[2];
  l4_umword_t label;
  l4_umword_t _res2[3];
  l4_msgtag_t tag;
} l4_vcpu_ipc_regs_t;
