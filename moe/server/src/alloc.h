/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/re/mem_alloc>
#include <l4/sys/cxx/ipc_epiface>
#include "quota.h"
#include "server_obj.h"

#include <l4/cxx/list>

#ifndef NDEBUG
#include <l4/re/debug>
typedef L4Re::Debug_obj_t<L4::Factory> Allocator_iface;
#else
typedef L4::Factory Allocator_iface;
#endif

namespace Moe {
class Dataspace;
}

class Allocator :
  public L4::Epiface_t<Allocator, Allocator_iface, Moe::Server_object>
{
private:
  Moe::Q_alloc _qalloc;
  long _sched_prio_limit;
  l4_umword_t _sched_cpu_mask;

public:
  explicit Allocator(size_t limit, unsigned prio_limit = 0)
  : _qalloc(limit), _sched_prio_limit(prio_limit), _sched_cpu_mask(~0UL)
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
                        unsigned long align = 0);

  /// Return the quota allocator that contains this factory.
  Moe::Q_alloc *parent_qalloc()
  {
    return dynamic_cast<Moe::Q_alloc *>(Moe::Malloc_container::from_ptr(this));
  }

  virtual ~Allocator();

  int op_create(L4::Factory::Rights rights, L4::Ipc::Cap<void> &, long,
                L4::Ipc::Varg_list<> &&args);

#ifndef NDEBUG
  long op_debug(L4Re::Debug_obj::Rights, unsigned long function);
#endif

  static Allocator *root_allocator();

};
