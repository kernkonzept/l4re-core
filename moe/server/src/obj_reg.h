/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/task.h>
#include <l4/sys/factory.h>
#include <l4/sys/types.h>
#include <l4/sys/thread>

#include <l4/re/util/bitmap_cap_alloc>
#include <l4/re/error_helper>
#include <l4/re/env>

#include <l4/cxx/exceptions>
#include <l4/cxx/hlist>

#include "server_obj.h"

#include <cstring>
#include <cassert>
#include <cstdio>

#define DEBUG_CAP_ALLOC 0

enum
{
  Rcv_cap = 0x100,
};

class Cap_alloc;

class Object_pool
: public L4::Basic_registry,
  private L4::Irqep_t<Object_pool>
{
  friend struct L4::Irqep_t<Object_pool>;

public:
  explicit Object_pool(Cap_alloc *ca);
  Cap_alloc *cap_alloc() const { return _cap_alloc; }
  cxx::H_list_t<Moe::Server_object> life;

private:
  Cap_alloc *_cap_alloc;

  void handle_irq()
  {
    l4_utcb_t *utcb = l4_utcb();
    for (auto i = life.begin(); i != life.end();)
      {
        if (i->obj_cap() && !i->obj_cap().validate(utcb).label())
          delete *i;
        else
          ++i;
      }
  }
};

class Cap_alloc
{
public:
  enum
  {
    Non_gc_caps = 8192,
    Non_gc_cap_0 = Rcv_cap + 1,
  };

private:
  // caps mainly used for things from outside (registered in name spaces)
  // this are usually not a lot
  L4Re::Util::Cap_alloc<Non_gc_caps> _non_gc;

public:
  Cap_alloc() : _non_gc(Non_gc_cap_0)
  {}

  L4::Cap<L4::Kobject> alloc()
  {
     L4::Cap<L4::Kobject> cap = _non_gc.alloc<L4::Kobject>();
#if DEBUG_CAP_ALLOC
     L4::cerr << "AC->" << L4::n_hex(cap.cap()) << "\n";
#endif
     return cap;
  }

  template< typename T >
  L4::Cap<T> alloc() { return L4::cap_cast<T>(alloc()); }

  L4::Cap<L4::Kobject> alloc(Moe::Server_object *_o)
  {
    // make sure we register an Epiface ptr
    L4::Epiface *o = _o;
    L4::Cap<L4::Kobject> cap = _non_gc.alloc<L4::Kobject>();
#if  DEBUG_CAP_ALLOC
    L4::cerr << "ACO->" << L4::n_hex(cap.cap()) << "\n";
#endif
    if (!cap.is_valid())
      throw(L4::Out_of_memory());

    l4_umword_t id = l4_umword_t(o);
    l4_factory_create_gate(L4_BASE_FACTORY_CAP, cap.cap(),
                           L4_BASE_THREAD_CAP, id);

    _o->set_obj_cap(cap);
    return cap;
  }

  bool free(L4::Cap<void> const &cap, l4_umword_t unmap_flags = L4_FP_ALL_SPACES)
  {
    if (!cap.is_valid())
      return false;

    if ((cap.cap() >> L4_CAP_SHIFT) >= Non_gc_cap_0)
      _non_gc.free(cap, L4_BASE_TASK_CAP, unmap_flags);
    else
      return false;

    return true;
  }

};

inline Object_pool::Object_pool(Cap_alloc *ca) : _cap_alloc(ca)
{
  // make sure we register an Epiface PTR
  L4::Epiface *self = this;
  auto c = L4Re::chkcap(cap_alloc()->alloc<L4::Irq>());
  L4Re::chksys(L4Re::Env::env()->factory()->create(c));
  L4Re::chksys(c->attach(l4_umword_t(self),
                         L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP)));
  set_obj_cap(c);
  L4Re::chksys(L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP)->register_del_irq(c));
}

