/*
 * Copyright (C) 2022-2025 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *            Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/**
 * \file
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

/**
 * \defgroup l4_thread_group_api Thread groups
 * \ingroup l4_kernel_object_api
 *
 * C thread group interface, see L4::Thread_group for the C++ interface.
 *
 * An L4 thread group is a collection of threads used as indirection for IPC
 * gate and IRQ objects such that these objects can have multiple receivers,
 * from which the kernel selects one according to a policy.
 *
 * The primary use case for thread groups are multi-threaded servers and
 * CPU core local IRQ / IPC delivery.
 *
 * A thread can be bound to at most one thread group. Before binding a thread to
 * a thread group, the thread must be bound to a task. All threads bound to the
 * same thread group must belong to the same task.
 */

enum L4_thread_group_ops
{
  L4_THREAD_GROUP_ADD_OP = 2UL,
  L4_THREAD_GROUP_REMOVE_OP = 3UL,
};

enum L4_thread_group_policy
{
  L4_THREAD_GROUP_POLICY_STRICT_CORE_LOCAL = 0,
  L4_THREAD_GROUP_POLICY_SOFT_CORE_LOCAL = 1,
};

/**
 * \ingroup l4_thread_group_api
 * \copybrief L4::Thread_group::add
 * \param tg  Thread group object.
 * \param thread  Thread to add to the thread group.
 */
L4_INLINE l4_msgtag_t
l4_thread_group_add(l4_cap_idx_t tg,
                    l4_cap_idx_t thread) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_thread_group_add_u(l4_cap_idx_t tg,
                      l4_cap_idx_t thread,
                      l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_thread_group_api
 * \copybrief L4::Thread_group::remove
 * \param tg  Thread group object.
 * \param thread  Thread to remove from the thread group.
 */
L4_INLINE l4_msgtag_t
l4_thread_group_remove(l4_cap_idx_t tg,
                       l4_cap_idx_t thread) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_thread_group_remove_u(l4_cap_idx_t tg,
                         l4_cap_idx_t thread,
                         l4_utcb_t *utcb) L4_NOTHROW;


/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_thread_group_add_u(l4_cap_idx_t tg,
                      l4_cap_idx_t thread,
                      l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_GROUP_ADD_OP;
  v->mr[1] = l4_map_obj_control(0, 0);
  v->mr[2] = l4_obj_fpage(thread, 0, L4_CAP_FPAGE_RWS).raw;
  return l4_ipc_call(tg, utcb,
                     l4_msgtag(L4_PROTO_THREAD_GROUP, 1, 1, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_group_remove_u(l4_cap_idx_t tg,
                         l4_cap_idx_t thread,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_GROUP_REMOVE_OP;
  v->mr[1] = l4_map_obj_control(0, 0);
  v->mr[2] = l4_obj_fpage(thread, 0, L4_CAP_FPAGE_RWS).raw;
  return l4_ipc_call(tg, utcb,
                     l4_msgtag(L4_PROTO_THREAD_GROUP, 1, 1, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_group_add(l4_cap_idx_t tg,
                    l4_cap_idx_t thread) L4_NOTHROW
{
  return l4_thread_group_add_u(tg, thread, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_thread_group_remove(l4_cap_idx_t tg,
                       l4_cap_idx_t thread) L4_NOTHROW
{
  return l4_thread_group_remove_u(tg, thread, l4_utcb());
}
