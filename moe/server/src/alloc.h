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

  Moe::Dataspace *alloc(long size, unsigned long flags = 0,
                        unsigned long align = 0,
                        Single_page_alloc_base::Config cfg
                          = Single_page_alloc_base::default_mem_cfg);

  virtual ~Allocator();

  int op_create(L4::Factory::Rights rights, L4::Ipc::Cap<void> &, long,
                L4::Ipc::Varg_list<> &&args);

  long op_info(L4Re::Mem_alloc::Rights right, L4Re::Mem_alloc::Stats &stats);

#ifndef NDEBUG
  long op_debug(L4Re::Debug_obj::Rights, unsigned long function);
#endif

  static Allocator *root_allocator();

};
