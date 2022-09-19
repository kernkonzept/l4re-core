/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>


/**
 * Set ASID of task.
 * \ingroup l4_platform_control_api
 *
 * On Cortex-R52 platforms, it might be necessary to control the VMID of a task
 * or virtual machine explicitly. The IOMPU on such platforms will use it for
 * further access control of device memory accesses. A privileged component
 * can use this call to control the value.
 *
 * The caller must have write permissions to the destination task.
 *
 * \param pfc     Capability selector for the platform-control object.
 * \param task    Capability selector of destination task
 * \param asid    New ASID value
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_platform_ctl_set_task_asid(l4_cap_idx_t pfc,
                              l4_cap_idx_t task,
                              l4_umword_t asid) L4_NOTHROW;


/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_platform_ctl_set_task_asid_u(l4_cap_idx_t pfc,
                                l4_cap_idx_t task,
                                l4_umword_t asid,
                                l4_utcb_t *utcb) L4_NOTHROW;

/* IMPLEMENTATION -----------------------------------------------------------*/

L4_INLINE l4_msgtag_t
l4_platform_ctl_set_task_asid_u(l4_cap_idx_t pfc,
                                l4_cap_idx_t task,
                                l4_umword_t asid,
                                l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_PLATFORM_CTL_SET_TASK_ASID_OP;
  v->mr[1] = asid;
  v->mr[2] = l4_map_obj_control(0,0);
  v->mr[3] = l4_obj_fpage(task, 0, L4_CAP_FPAGE_RWS).raw;
  return l4_ipc_call(pfc, utcb, l4_msgtag(L4_PROTO_PLATFORM_CTL, 2, 1, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_platform_ctl_set_task_asid(l4_cap_idx_t pfc,
                              l4_cap_idx_t task,
                              l4_umword_t asid) L4_NOTHROW
{
  return l4_platform_ctl_set_task_asid_u(pfc, task, asid, l4_utcb());
}
