/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/exceptions>
#include <l4/cxx/unique_ptr>
#include <l4/cxx/l4iostream>

#include <l4/re/util/meta>
#include <l4/sys/factory>

#include <cstdlib>
#include <climits>

#include "debug.h"
#include "alloc.h"
#include "dataspace_anon.h"
#include "dataspace_noncont.h"
#include "dma_space.h"
#include "globals.h"
#include "page_alloc.h"
#include "quota.h"
#include "region.h"
#include "name_space.h"
#include "log.h"
#include "sched_proxy.h"

static Dbg dbg(Dbg::Warn | Dbg::Server);

Moe::Dataspace *
Allocator::alloc(long size, unsigned long flags, unsigned long align)
{
  if (size == 0)
    throw L4::Bounds_error("stack too small");

  //L4::cout << "A: \n";
  Moe::Dataspace *mo;
  if (flags & L4Re::Mem_alloc::Continuous
      || flags & L4Re::Mem_alloc::Pinned)
    {
      if (flags & L4Re::Mem_alloc::Super_pages)
        align = cxx::max<unsigned long>(align, L4_SUPERPAGESHIFT);
      else
        align = cxx::max<unsigned long>(align, L4_PAGESHIFT);

      mo = make_obj<Moe::Dataspace_anon>(size, true, align);
    }
  else
    {
      if (size < 0)
        throw L4::Bounds_error("invalid size");

      mo = Moe::Dataspace_noncont::create(qalloc(), size);
      Obj_list::insert_after(mo, Obj_list::iter(this));
    }

  // L4::cout << "A: mo=" << mo << "\n";

  //L4::cout << "A[" << this << "]: allocated(" << mo << " " << mo->obj_cap() << ")\n";
  return mo;
}

Allocator::~Allocator()
{
  assert (Obj_list::in_list(this));

  // NOTE: the Obj_list iterator must be/is safe according to deletion of the
  // current or any later element from the list, when deleting the current
  // element the iterator automatically advances to the next element.
  auto it = ++Obj_list::iter(this);
  while (it != Obj_list::Iterator()
         && Moe::Malloc_container::from_ptr(*it) == qalloc())
    delete *it;

  if (qalloc()->quota()->used() > 0)
    dbg.printf("WARNING: destroyed allocator still holds resources.");

  // return our quota to the parant
  qalloc()->reparent(parent_qalloc());
}


class LLog : public Moe::Log
{
private:
  char _tag[32];

public:
  LLog(char const *t, int l, unsigned char col) : Log()
  {
    if (l > 32)
      l = 32;

    memcpy(_tag, t, l);

    set_tag(_tag, l);

    set_color(col);
  }

  virtual ~LLog() {}
};

int
Allocator::op_create(L4::Factory::Rights, L4::Ipc::Cap<void> &res,
                     long type, L4::Ipc::Varg_list<> &&args)
{
  L4::Cap<L4::Kobject> ko;

  switch (type)
    {
    case L4Re::Namespace::Protocol:
        {
          cxx::unique_ptr<Moe::Name_space> o(make_obj<Moe::Name_space>());
          ko = object_pool.cap_alloc()->alloc(o.get());
          ko->dec_refcnt(1);
          o.release();
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
          return L4_EOK;
        }

    case L4Re::Rm::Protocol:
        {
          cxx::unique_ptr<Region_map> o(make_obj<Region_map>());
          ko = object_pool.cap_alloc()->alloc(o.get());
          ko->dec_refcnt(1);
          o.release();
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
          return L4_EOK;
        }

    case L4::Factory::Protocol:
        {
          L4::Ipc::Varg tag = args.next();

          if (!tag.is_of_int() || tag.value<long>() == 0) // ignore sign
            return -L4_EINVAL;
          Moe::Quota_guard g(_qalloc.quota(), tag.value<long>());
          cxx::unique_ptr<Allocator> o(make_obj<Allocator>(tag.value<long>()));
          ko = object_pool.cap_alloc()->alloc(o.get());
          ko->dec_refcnt(1);
          o.release();
          g.release();
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);

          return 0;
        }

    case L4_PROTO_LOG:
        {
          L4::Ipc::Varg tag = args.next();

          if (!tag.is_of<char const *>())
            return -L4_EINVAL;

          L4::Ipc::Varg col = args.next();

          int color;
          if (col.is_of<char const *>())
            color = LLog::color_value(cxx::String(col.value<char const *>(),
                                      col.length() - 1));
          else if (col.is_of_int())
            color = col.value<l4_mword_t>();
          else
            color = 7;

          cxx::unique_ptr<Moe::Log> l(make_obj<LLog>(tag.value<char const *>(),
                                                     tag.length() - 1, color));
          ko = object_pool.cap_alloc()->alloc(l.get());
          ko->dec_refcnt(1);
          l.release();
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
          return L4_EOK;
        }

    case L4::Scheduler::Protocol:
        {
          if (!_sched_prio_limit)
            return -L4_ENODEV;

          L4::Ipc::Varg p_max = args.next(), p_base = args.next(), cpus = args.next();

          if (!p_max.is_of_int() || !p_base.is_of_int())
            return -L4_EINVAL;

          if (p_max.value<l4_mword_t>() > _sched_prio_limit
              || p_base.value<l4_mword_t>() > _sched_prio_limit)
            return -L4_ERANGE;

          if (p_max.value<l4_mword_t>() <= p_base.value<l4_mword_t>())
            return -L4_EINVAL;

          l4_umword_t cpu_mask = ~0UL;

          if (!cpus.is_of<void>() && cpus.is_of_int())
            cpu_mask = cpus.value<l4_umword_t>();

          cxx::unique_ptr<Sched_proxy> o(make_obj<Sched_proxy>());
          o->set_prio(p_base.value<l4_mword_t>(), p_max.value<l4_mword_t>());
          o->restrict_cpus(cpu_mask);
          ko = object_pool.cap_alloc()->alloc(o.get());
          ko->dec_refcnt(1);
          o.release();
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
          return L4_EOK;
        }

    case L4Re::Dataspace::Protocol:
        {
          L4::Ipc::Varg size = args.next(), flags = args.next(), align = args.next();

          if (!size.is_of_int())
            return -L4_EINVAL;

          // L4::cout << "MEM: alloc ... " << size.value<l4_mword_t>() << "; " << flags.value<l4_umword_t>() << "\n";
          cxx::unique_ptr<Moe::Dataspace> mo(alloc(size.value<l4_mword_t>(),
                flags.is_of_int() ? flags.value<l4_umword_t>() : 0,
                align.is_of_int() ? align.value<l4_umword_t>() : 0));

          // L4::cout << "MO=" << mo.get() << "\n";
          ko = object_pool.cap_alloc()->alloc(mo.get());
          ko->dec_refcnt(1);
          // L4::cout << "MO_CAP=" << mo->obj_cap() << "\n";
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
          mo.release();
          return L4_EOK;
        }

    case L4Re::Dma_space::Protocol:
        {
          cxx::unique_ptr<Moe::Dma_space> o(make_obj<Moe::Dma_space>());
          ko = object_pool.cap_alloc()->alloc(o.get());
          ko->dec_refcnt(1);
          res = L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
          o.release();
          return L4_EOK;
        }

    default:
      return -L4_ENODEV;
    }
}

#ifndef NDEBUG
long
Allocator::op_debug(L4Re::Debug_obj::Rights, unsigned long)
{
  Dbg out(Dbg::Info, "mem_alloc");
  out.printf("quota (bytes): limit=%zd, used=%zd avail=%zd\n",
             _qalloc.quota()->limit(), _qalloc.quota()->used(),
             _qalloc.quota()->limit() == ~0u
             ? -1 : _qalloc.quota()->limit() - _qalloc.quota()->used());
  out.printf("global: avail=%lu bytes\n", Single_page_alloc_base::_avail());
  out.printf("global physical free list:\n");
  Single_page_alloc_base::_dump_free(out);
  return L4_EOK;
}
#endif

static Allocator *_root_alloc;

Allocator *
Allocator::root_allocator()
{
  if (_root_alloc)
    return _root_alloc;

  _root_alloc = Moe::Moe_alloc::allocator()->make_obj<Allocator>(~0, 300000);
  object_pool.life.push_front(_root_alloc);
  return _root_alloc;
}
