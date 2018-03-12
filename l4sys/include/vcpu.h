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
#include <l4/sys/__vcpu-arch.h>

/**
 * \defgroup l4_vcpu_api vCPU API
 * \ingroup  l4_thread_api
 * vCPU API
 */

/**
 * State of a vCPU
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_state_t
{
  l4_umword_t          version;
  l4_umword_t          user_data[7];
  l4_vcpu_regs_t       r;             ///< Register state
  l4_vcpu_ipc_regs_t   i;             ///< IPC state

  l4_uint16_t          state;         ///< Current vCPU state
  l4_uint16_t          saved_state;   ///< Saved vCPU state
  l4_uint16_t          sticky_flags;  ///< Pending flags
  l4_uint16_t          _reserved;     ///< \internal

  l4_cap_idx_t         user_task;     ///< User task to use

  l4_umword_t          entry_sp;      ///< Stack pointer for entry (when coming from user task)
  l4_umword_t          entry_ip;      ///< IP for entry
  l4_umword_t          reserved_sp;   ///< \internal
  l4_vcpu_arch_state_t arch_state;
} l4_vcpu_state_t;

/**
 * State flags of a vCPU
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_state_flags
{
  L4_VCPU_F_IRQ         = 0x01, ///< IRQs (events) enabled
  L4_VCPU_F_PAGE_FAULTS = 0x02, ///< Page faults enabled
  L4_VCPU_F_EXCEPTIONS  = 0x04, ///< Exception enabled
  L4_VCPU_F_DEBUG_EXC   = 0x08, ///< Debug exception enabled
  L4_VCPU_F_USER_MODE   = 0x20, ///< User task will be used
  L4_VCPU_F_FPU_ENABLED = 0x80, ///< FPU enabled
};

/**
 * Sticky flags of a vCPU
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_sticky_flags
{
  L4_VCPU_SF_IRQ_PENDING = 0x01, ///< An event (e.g. IRQ) is pending
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
 * Check if a vCPU state has the right version.
 *
 * \param  vcpu  A pointer to an initialized vCPU state.
 *
 * \retval 1  If the vCPU state has a matching version ID for the current
 *            user-level structures.
 * \retval 0  If the vCPU state has a different (incompatible) version ID than
 *            the current vCPU user-level structures.
 *
 */
L4_INLINE int
l4_vcpu_check_version(l4_vcpu_state_t const *vcpu) L4_NOTHROW;

/* IMPLEMENTATION: ------------------------------------------------*/

L4_INLINE int
l4_vcpu_check_version(l4_vcpu_state_t const *vcpu) L4_NOTHROW
{
  return vcpu->version == L4_VCPU_STATE_VERSION;
}
