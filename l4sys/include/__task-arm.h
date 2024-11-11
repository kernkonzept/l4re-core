/*
 * (c) 2018 Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_vgicc_map_u(l4_cap_idx_t task, l4_fpage_t vgicc_fpage,
                   l4_utcb_t *u) L4_NOTHROW;

/**
 * Map the GIC virtual CPU interface page to the task in case Fiasco
 * detected a GIC version 2.
 * \ingroup l4_task_api
 *
 * \param task          Capability selector of destination task
 * \param vgicc_fpage   Flexpage that describes an area in the address space
 *                      of the destination task to map the vGICC page to
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_task_vgicc_map(l4_cap_idx_t task, l4_fpage_t vgicc_fpage) L4_NOTHROW;

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_task_vgicc_map_u(l4_cap_idx_t task, l4_fpage_t vgicc_fpage,
                   l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  v->mr[0] = L4_TASK_MAP_VGICC_ARM_OP;
  v->mr[1] = vgicc_fpage.raw;
  return l4_ipc_call(task, u, l4_msgtag(L4_PROTO_TASK, 2, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_task_vgicc_map(l4_cap_idx_t task, l4_fpage_t vgicc_fpage) L4_NOTHROW
{
  return l4_task_vgicc_map_u(task, vgicc_fpage, l4_utcb());
}
