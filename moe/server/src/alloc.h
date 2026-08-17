/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/re/mem_alloc>
#include <l4/sys/cxx/ipc_epiface>
#include "quota.h"
#include "server_obj.h"

#include <l4/cxx/list>

#ifndef NDEBUG
#include <l4/re/debug>
typedef L4Re::Debug_obj_t<L4Re::Mem_alloc> Allocator_iface;
#else
typedef L4Re::Mem_alloc Allocator_iface;
#endif

namespace Moe {
class Dataspace;
}

class Allocator :
  public L4::Epiface_t<Allocator, Allocator_iface, Moe::Server_object>
{
private:
  enum : l4_mword_t { Max_priority = 255 };

  Moe::Q_alloc _qalloc;
  bool _is_root;  ///< Is this the root allocator?

public:
  explicit Allocator(Moe::Quota *parent, size_t limit, bool is_root = false)
  : _qalloc(parent, limit), _is_root(is_root)
  {}

  template<typename T, typename ...ARGS>
  T *make_obj(ARGS &&...args)
  {
    T *o = qalloc()->make_obj<T>(cxx::forward<ARGS>(args)...);
    Obj_list::insert_after(o, Obj_list::iter(this));
    return o;
  }

  Moe::Q_alloc *qalloc() { return &_qalloc; }

  Moe::Dataspace *alloc(long size, Single_page_alloc_base::Config cfg,
                        unsigned long flags = 0, unsigned long align = 0);

  virtual ~Allocator();

  l4_ret_t op_create(L4::Factory::Rights rights, L4::Ipc::Cap<void> &, long,
                     L4::Ipc::Varg_list<> &&args);

  l4_ret_t op_info(L4Re::Mem_alloc::Rights right, L4Re::Mem_alloc::Stats &stats);

#ifndef NDEBUG
  l4_ret_t op_debug(L4Re::Debug_obj::Rights, unsigned long function);
#endif

  static Allocator *root_allocator();

private:
  l4_ret_t create_namespace(L4::Ipc::Cap<void> &res);
  l4_ret_t create_rm(L4::Ipc::Cap<void> &res);
  l4_ret_t create_factory(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args);
  l4_ret_t create_log(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args);
  l4_ret_t create_scheduler(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args);
  l4_ret_t create_dataspace(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args);
  l4_ret_t create_dma_space(L4::Ipc::Cap<void> &res);
};
