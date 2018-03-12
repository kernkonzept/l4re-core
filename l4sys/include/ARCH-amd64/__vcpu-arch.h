/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/types.h>

enum
{
  /**
   * Architecture specific version ID.
   *
   * This ID must match the version field in the l4_vcpu_state_t structure
   * after enabling vCPU mode or extended vCPU mode for a thread.
   */
  L4_VCPU_STATE_VERSION = 0x23
};

typedef struct l4_vcpu_arch_state_t
{
  l4_umword_t host_fs_base;
  l4_umword_t host_gs_base;
  l4_uint16_t host_ds, host_es, host_fs, host_gs;

  l4_uint16_t const user_ds32;
  l4_uint16_t const user_cs64;
  l4_uint16_t const user_cs32;
} l4_vcpu_arch_state_t;


/**
 * \brief vCPU registers.
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_regs_t
{
  l4_umword_t r15;     /**< r15 register */
  l4_umword_t r14;     /**< r14 register */
  l4_umword_t r13;     /**< r13 register */
  l4_umword_t r12;     /**< r12 register */
  l4_umword_t r11;     /**< r11 register */
  l4_umword_t r10;     /**< r10 register */
  l4_umword_t r9;      /**< r9 register */
  l4_umword_t r8;      /**< r8 register */

  l4_umword_t di;      /**< rdi register */
  l4_umword_t si;      /**< rsi register */
  l4_umword_t bp;      /**< rbp register */
  l4_umword_t pfa;     /**< page fault address */
  l4_umword_t bx;      /**< rbx register */
  l4_umword_t dx;      /**< rdx register */
  l4_umword_t cx;      /**< rcx register */
  l4_umword_t ax;      /**< rax register */

  l4_umword_t trapno;  /**< trap number */
  l4_umword_t err;     /**< error code */

  l4_umword_t ip;      /**< instruction pointer */
  l4_umword_t cs;      /**< dummy \internal */
  l4_umword_t flags;   /**< eflags */
  l4_umword_t sp;      /**< stack pointer */
  l4_umword_t ss;
  l4_umword_t fs_base;
  l4_umword_t gs_base;
  l4_uint16_t ds, es, fs, gs;

} l4_vcpu_regs_t;

/**
 * \brief vCPU message registers.
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_ipc_regs_t
{
  l4_umword_t _res[1];
  l4_umword_t label;
  l4_umword_t _res2[5];
  l4_msgtag_t tag;
} l4_vcpu_ipc_regs_t;
