/*
 * Copyright (C) 2023 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Debugger related functions.
 * \ingroup api_calls_fiasco
 * \attention This API is subject to change!
 */

#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/debugger.h>
#include <l4/sys/l4int.h>

struct L4_kobj_info
{
  // Type_mapping: See Jdb_mapdb::info_obj_mapping().
  struct Mapping
  {
    enum { Type = 0 };
    l4_uint64_t mapping_ptr;
    char space_name[16];
    l4_uint32_t cap_idx;
    l4_uint16_t entry_rights;
    l4_uint16_t entry_flags;
    l4_uint64_t entry_ptr;
  };

  // Type_thread: See Jdb_tcb::info_kobject().
  struct Thread
  {
    enum { Type = 1 };
    bool is_kernel;
    bool is_current;
    bool in_ready_list;
    bool is_kernel_task;
    l4_uint32_t home_cpu;
    l4_uint32_t current_cpu;
    l4_int64_t ref_cnt;
    l4_uint64_t space_id;
  };

  // Type_space: See Jdb_space::info_kobject().
  struct Space
  {
    enum { Type = 2 };
    bool is_kernel;
    l4_int64_t ref_cnt;
  };

  // Type_vm: See Jdb_vm::info_kobject().
  struct Vm
  {
    enum { Type = 3 };
    l4_uint64_t utcb;
    l4_uint64_t pc;
  };

  // Type_ipc_gate: See Jdb_ipc_gate::info_kobject().
  struct Ipc_gate
  {
    enum { Type = 4 };
    l4_uint64_t label;
    l4_uint64_t thread_id;
  };

  // Type_irq: See Jdb_kobject_irq::info_kobject().
  struct Irq_sender
  {
    enum { Type = 5 };
    char chip_type[10];
    l4_uint16_t flags;
    l4_uint32_t pin;
    l4_uint64_t label;
    l4_uint64_t target_id;
    l4_int64_t queued;
  };

  // Type_irq: See Jdb_kobject_irq::info_kobject().
  struct Irq_semaphore
  {
    enum { Type = 6 };
    char chip_type[10];
    l4_uint16_t flags;
    l4_uint32_t pin;
    l4_uint64_t sender_id;
    l4_uint64_t target_id;
    l4_int64_t queued;
  };

  // Type_factory: See Jdb_factory::info_kobject().
  struct Factory
  {
    enum { Type = 7 };
    l4_uint64_t current;
    l4_uint64_t limit;
  };

  struct Jdb         { enum { Type =  8 }; };
  struct Scheduler   { enum { Type =  9 }; };
  struct Vlog        { enum { Type = 10 }; };
  struct Pfc         { enum { Type = 11 }; };
  struct Dmar_space  { enum { Type = 12 }; };
  struct Iommu       { enum { Type = 13 }; };
  struct Smmu        { enum { Type = 14 }; };

  l4_uint64_t type:5;
  l4_uint64_t id:59;
  l4_uint64_t mapping_ptr;
  l4_uint64_t ref_cnt;
  union
  {
    Thread thread;
    Space space;
    Vm vm;
    Ipc_gate ipc_gate;
    Irq_sender irq_sender;
    Irq_semaphore irq_semaphore;
    Factory factory;
    Mapping mapping;
    l4_uint64_t raw[5];
  };
};

static_assert(sizeof(L4_kobj_info) == 64, "Size of Jobj_info");

/**
 * Retrieve information from the kernel about all objects in the mapping
 * database and write data to the passed KU memory.
 *
 * \param cap              Capability of the debugger object.
 * \param ku_mem_addr      Address of the KU memory for writing the information.
 * \param ku_mem_size      Size of the KU memory to writing the information.
 * \param skip             Number of objects to skip.
 * \param[out] result_cnt  Number of objects in the mapping database.
 * \param[out] result_all  Number of objects written to the KU memory.
 *
 * \note The kernel will only write a number of object information which fits
 *       to the passed KU memory. To retrieve missing object information,
 *       repeat the call and adapt the `skip` parameter accordingly.
 *
 * \note If this system call is performed several times, the number of kernel
 *       objects might have changed in the meantime.
 */
L4_INLINE l4_msgtag_t
l4_debugger_query_obj_infos(l4_cap_idx_t cap, l4_addr_t ku_mem_addr,
                            l4_size_t ku_mem_size, l4_umword_t skip,
                            l4_umword_t *result_cnt, l4_umword_t *result_all)
                            L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_debugger_query_obj_infos_u(l4_cap_idx_t cap, l4_addr_t ku_mem_addr,
                              l4_size_t ku_mem_size, l4_umword_t skip,
                              l4_umword_t *result_cnt, l4_umword_t *result_all,
                              l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr()->mr[0] = L4_DEBUGGER_OBJ_INFO_OP;
  l4_utcb_mr()->mr[1] = ku_mem_addr;
  l4_utcb_mr()->mr[2] = ku_mem_size;
  l4_utcb_mr()->mr[3] = skip;

  l4_msgtag_t tag = l4_invoke_debugger(cap, l4_msgtag(0, 4, 0, 0), utcb);

  *result_cnt = l4_utcb_mr()->mr[0];
  *result_all = l4_utcb_mr()->mr[1];

  return tag;
}

L4_INLINE l4_msgtag_t
l4_debugger_query_obj_infos(l4_cap_idx_t cap, l4_addr_t ku_mem_addr,
                            l4_size_t ku_mem_size, l4_umword_t skip,
                            l4_umword_t *result_cnt, l4_umword_t *result_all)
                            L4_NOTHROW
{
  return l4_debugger_query_obj_infos_u(cap, ku_mem_addr, ku_mem_size, skip,
                                       result_cnt, result_all, l4_utcb());
}
