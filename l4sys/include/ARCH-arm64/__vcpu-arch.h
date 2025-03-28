/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/**
 * \file
 * ARM64-specific vCPU interface.
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>
#include <l4/sys/__vcpu-arm.h>

enum
{
  /**
   * Architecture-specific version ID.
   *
   * This ID must match the version field in the l4_vcpu_state_t structure
   * after enabling vCPU mode or extended vCPU mode for a thread.
   */
  L4_VCPU_STATE_VERSION = 0x38,

  L4_VCPU_STATE_SIZE = 0x200,
  L4_VCPU_STATE_EXT_SIZE = 0x800,
};

/**
 * Offsets for vCPU state layouts
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_state_offset
{
  L4_VCPU_OFFSET_EXT_STATE = 0x280, ///< Offset where extended state begins
  L4_VCPU_OFFSET_EXT_INFOS = 0x200, ///< Offset where extended infos begin
};

L4_INLINE l4_arm_vcpu_e_info_t const *
l4_vcpu_e_info(void const *vcpu) L4_NOTHROW
{
  return (l4_arm_vcpu_e_info_t const *)((l4_addr_t)vcpu
                                        + L4_VCPU_OFFSET_EXT_INFOS);
}

L4_INLINE void *l4_vcpu_e_ptr(void const *vcpu, unsigned id) L4_NOTHROW
{ return (void *)((l4_addr_t)vcpu + L4_VCPU_OFFSET_EXT_STATE + (id & 0xfff)); }

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
  l4_umword_t host_tpidruro;
} l4_vcpu_arch_state_t;

/**
 * \brief vCPU message registers.
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_ipc_regs_t
{
  l4_msgtag_t tag;
  l4_umword_t label;
  l4_umword_t _d1[3];
} l4_vcpu_ipc_regs_t;

/**
 * IDs for extended vCPU state fields.
 *
 * Bits 14..15: are the field size:
 *  * 0 = 32bit field
 *  * 1 = register width field
 *  * 2 = 64bit field
 */
enum L4_vcpu_e_field_ids
{
  L4_VCPU_E_HCR        = 0x8008,
  L4_VCPU_E_SCTLR      = 0x0010,
  L4_VCPU_E_CPACR      = 0x0014,

  L4_VCPU_E_CNTVCTL    = 0x0018,
  L4_VCPU_E_CNTVOFF    = 0x8020,

  L4_VCPU_E_VMPIDR     = 0x8028,
  L4_VCPU_E_VPIDR      = 0x0030,

  L4_VCPU_E_VTMR_CFG   = 0x0034,  ///< vtimer irq configuration
  L4_VCPU_E_VTCR       = 0x8038,

  L4_VCPU_E_GIC_HCR    = 0x0040,
  L4_VCPU_E_GIC_VTR    = 0x0044,
  L4_VCPU_E_GIC_VMCR   = 0x0048,
  L4_VCPU_E_GIC_MISR   = 0x004c,
  L4_VCPU_E_GIC_EISR   = 0x0050,
  L4_VCPU_E_GIC_ELSR   = 0x0054,
  L4_VCPU_E_GIC_V2_LR0 = 0x0058,
  L4_VCPU_E_GIC_V3_LR0 = 0x8058,
};
