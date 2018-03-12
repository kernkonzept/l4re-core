/**
 * \file
 * \brief ARM virtualization interface.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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

/**
 * \defgroup l4_vm_tz_api VM API for TZ
 * \brief Virtual Machine API for ARM TrustZone
 * \ingroup l4_vm_api
 */

/**
 * \internal
 * \ingroup l4_vm_tz_api
 */
struct l4_vm_tz_state_mode
{
  l4_umword_t sp;
  l4_umword_t lr;
  l4_umword_t spsr;
};

struct l4_vm_tz_state_irq_inject
{
  l4_uint32_t group;
  l4_uint32_t irqs[8];
};

/**
 * \brief state structure for TrustZone VMs
 * \ingroup l4_vm_tz_api
 */
struct l4_vm_tz_state
{
  l4_umword_t r[13]; // r0 - r12

  l4_umword_t sp_usr;
  l4_umword_t lr_usr;

  struct l4_vm_tz_state_mode irq;

  l4_umword_t r_fiq[5]; // r8 - r12
  struct l4_vm_tz_state_mode fiq;
  struct l4_vm_tz_state_mode abt;
  struct l4_vm_tz_state_mode und;
  struct l4_vm_tz_state_mode svc;

  l4_umword_t pc;
  l4_umword_t cpsr;

  l4_umword_t pending_events;
  l4_uint32_t cpacr;
  l4_umword_t cp10_fpexc;

  l4_umword_t pfs;
  l4_umword_t pfa;
  l4_umword_t exit_reason;

  struct l4_vm_tz_state_irq_inject irq_inject;
};

enum L4_vm_exit_reason
{
  L4_vm_exit_reason_vmm_call   = 1,
  L4_vm_exit_reason_inst_abort = 2,
  L4_vm_exit_reason_data_abort = 3,
  L4_vm_exit_reason_irq        = 4,
  L4_vm_exit_reason_fiq        = 5,
  L4_vm_exit_reason_undef      = 6,
};

L4_INLINE int
l4_vm_tz_irq_inject(l4_vm_tz_state *state, unsigned irq);

L4_INLINE int
l4_vm_tz_irq_inject(l4_vm_tz_state *state, unsigned irq)
{
  if (irq > sizeof(state->irq_inject.irqs) * 8)
    return -L4_EINVAL;

  unsigned g = irq / 32;
  state->irq_inject.group   |= 1 << g;
  state->irq_inject.irqs[g] |= 1 << (irq & 31);

  return 0;
}
