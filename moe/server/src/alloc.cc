/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/cxx/exceptions>
#include <l4/cxx/unique_ptr>
#include <l4/cxx/l4iostream>
#include <l4/cxx/string>

#include <l4/bid_config.h>
#include <l4/re/util/meta>
#include <l4/sys/factory>

#include <cstdlib>
#include <climits>
#include <limits>

#include "debug.h"
#include "alloc.h"
#include "dataspace_anon.h"
#include "dataspace_noncont.h"
#include "dma_space.h"
#include "mem_region_list.h"
#include "factory_config.h"
#include "globals.h"
#include "page_alloc.h"
#include "quota.h"
#include "region.h"
#include "name_space.h"
#include "log.h"
#include "sched_proxy.h"

static Dbg dbg(Dbg::Warn | Dbg::Server);

Moe::Dataspace *
Allocator::alloc(long size, Single_page_alloc_base::Config cfg,
                 unsigned long flags, unsigned long align)
{
  if (size == 0)
    throw L4::Bounds_error("stack too small");

#if !defined(CONFIG_MMU)
  flags |= L4Re::Mem_alloc::Continuous | L4Re::Mem_alloc::Pinned;
#endif

  //L4::cout << "A: \n";
  Moe::Dataspace *mo;
  if (flags & L4Re::Mem_alloc::Continuous
      || flags & L4Re::Mem_alloc::Pinned)
    {
      if (flags & L4Re::Mem_alloc::Super_pages)
        align = cxx::max<unsigned long>(align, L4_SUPERPAGESHIFT);
      else
        align = cxx::max<unsigned long>(align, L4_PAGESHIFT);

      mo = make_obj<Moe::Dataspace_anon>(size, cfg, L4Re::Dataspace::F::RWX,
                                         align);
    }
  else
    {
      if (size < 0)
        throw L4::Bounds_error("invalid size");

      mo = Moe::Dataspace_noncont::create(qalloc(), size, cfg);
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

template<typename T>
static L4::Ipc::Cap<L4::Kobject>
register_ipc_object(cxx::unique_ptr<T> object, char const *name)
{
  auto ko = object_pool.cap_alloc()->alloc(object.get(), name);
  ko->dec_refcnt(1);
  object.release();
  return L4::Ipc::make_cap(ko, L4_CAP_FPAGE_RWSD);
}

l4_ret_t
Allocator::op_create(L4::Factory::Rights, L4::Ipc::Cap<void> &res,
                     long type, L4::Ipc::Varg_list<> &&args)
{
  switch (type)
    {
    case L4Re::Namespace::Protocol:
      return create_namespace(res);
    case L4Re::Rm::Protocol:
      return create_rm(res);
    case L4::Factory::Protocol:
      return create_factory(res, args);
    case L4_PROTO_LOG:
      return create_log(res, args);
    case L4::Scheduler::Protocol:
      return create_scheduler(res, args);
    case L4Re::Dataspace::Protocol:
      return create_dataspace(res, args);
    case L4Re::Dma_space::Protocol:
      return create_dma_space(res);
    default:
      return -L4_ENODEV;
    }
}

l4_ret_t
Allocator::create_namespace(L4::Ipc::Cap<void> &res)
{
  cxx::unique_ptr<Moe::Name_space> o(make_obj<Moe::Name_space>());
  res = register_ipc_object(cxx::move(o), "moe-ns");
  return L4_EOK;
}

l4_ret_t
Allocator::create_rm(L4::Ipc::Cap<void> &res)
{
  cxx::unique_ptr<Region_map> o(make_obj<Region_map>());
  res = register_ipc_object(cxx::move(o), "moe-rm");
  return L4_EOK;
}

l4_ret_t
Allocator::create_factory(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args)
{
  L4::Ipc::Varg quota = args.pop_front();

  if (!quota.is_of_int() || quota.value<long>() <= 0)
    return -L4_EINVAL;

  Moe::Factory_config sub_config;
  for (L4::Ipc::Varg arg : args)
    {
      if (!arg.is_of<char const *>())
        return -L4_EINVAL; // Unexpected argument.

      cxx::String arg_str = cxx::String(arg.value<char const *>(),
                                        arg.length() - 1);
      if (!Moe::parse_factory_config(arg_str, &sub_config))
        return -L4_EINVAL;
    }

  if (!sub_config.validate(_config))
    return -L4_EPERM;

  // Unless explicitly specified, a sub-factory inherits the untyped
  // allocation regions from the parent factory.
  if (sub_config.regions.empty())
    for (auto const &region : _config.untyped_regions())
      static_cast<void>(sub_config.regions.add(region));

  // ensure that 0 cannot be reached by an integer overflow when
  // converting long to size_t since size_t is used internally
  static_assert(   std::numeric_limits<long>::max()
                <= std::numeric_limits<size_t>::max(),
                "size_t must be able to hold the maximum of a long");
  cxx::unique_ptr<Allocator>
    o(make_obj<Allocator>(_qalloc.quota(), quota.value<long>(), sub_config));
  res = register_ipc_object(cxx::move(o), "moe-fact");

  return L4_EOK;
}

l4_ret_t
Allocator::create_log(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args)
{
  L4::Ipc::Varg tag = args.pop_front();

  if (!tag.is_of<char const *>())
    return -L4_EINVAL;

  L4::Ipc::Varg col = args.pop_front();

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
  res = register_ipc_object(cxx::move(l), "moe-log");
  return L4_EOK;
}

l4_ret_t
Allocator::create_scheduler(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args)
{
  if (!_config.has_permission(Moe::Factory_config::Scheduler_proxy))
    return -L4_ENODEV;

  L4::Ipc::Varg p_max  = args.pop_front(),
                p_base = args.pop_front(),
                cpus   = args.pop_front();

  if (!p_max.is_of_int() || !p_base.is_of_int())
    return -L4_EINVAL;

  if (p_max.value<l4_mword_t>() > Max_priority
      || p_base.value<l4_mword_t>() > Max_priority)
    return -L4_ERANGE;

  if (p_max.value<l4_mword_t>() <= p_base.value<l4_mword_t>())
    return -L4_EINVAL;

  unsigned cpu_mask_offs = 0;
  Dyn_cpu_set cpu_mask(qalloc());

  while (!cpus.is_of<void>())
    {
      if (!cpus.is_of_int())
        return -L4_EINVAL;
      cpu_mask.update(cpu_mask_offs, cpus.value<l4_umword_t>());
      cpu_mask_offs += sizeof(l4_umword_t) * 8;
      cpus = args.pop_front();
    }

  cxx::unique_ptr<Sched_proxy> o(make_obj<Sched_proxy>(qalloc()));
  o->set_prio(p_base.value<l4_mword_t>(), p_max.value<l4_mword_t>());
  if (cpu_mask_offs)
    o->restrict_cpus(cpu_mask);
  res = register_ipc_object(cxx::move(o), "moe-sched");
  return L4_EOK;
}

l4_ret_t
Allocator::create_dataspace(L4::Ipc::Cap<void> &res, L4::Ipc::Varg_list<> &args)
{
  L4::Ipc::Varg size  = args.pop_front(),
                flags = args.pop_front(),
                align = args.pop_front();

  if (!size.is_of_int())
    return -L4_EINVAL;

  l4_umword_t flags_val = flags.is_of_int() ? flags.value<l4_umword_t>() : 0;
  L4::Ipc::Varg base = L4::Ipc::Varg::nil();
  if (flags_val & L4Re::Mem_alloc::Fixed_paddr)
    base = args.pop_front();

  L4::Ipc::Varg type = args.pop_front();
  if (!type.is_nil() && !type.is_of<char const *>())
    return -L4_EINVAL;

  Single_page_alloc_base::Config ds_config;
  if (type.is_nil())
    {
      // For untyped allocations the untyped regions of the factory apply.
      ds_config.regions = _config.untyped_regions();
    }
  else
    {
      cxx::String type_str(type.value<char const *>(), type.length() - 1);
      if (!type_str.starts_with("type="))
        return -L4_EINVAL;
      type_str = type_str.substr(5);

      // Lookup regions for given type name. (the Dataspace references
      // regions in its factory, which is fine because a factory always
      // outlives the objects it created)
      ds_config.regions = _config.regions_with_type(type_str);
    }

  if (ds_config.regions.empty())
    return -L4_ENOMEM;

  Moe::Factory_config::Region_list ds_cont_regions;
#ifdef CONFIG_MMU
  // On MMU systems, the physical address is none of the clients
  // business.
  if (!base.is_nil())
    return -L4_EINVAL;
#else
  // On no-MMU systems, we must allow the caller to specify the base
  // address of the allocated memory. Otherwise the ELF loader and
  // guest RAM allocation won't work.
  if (base.is_of_int())
    {
      l4_addr_t physmin = base.value<l4_umword_t>();
      l4_addr_t physmax = physmin + size.value<l4_umword_t>() - 1U;
      if (physmin >= physmax)
        return -L4_EINVAL; // overflow

      // Check that the requested physical address range is covered by
      // the factory constraints.
      Moe::Mem_range alloc_range(physmin, physmax);
      if (!ds_config.regions.contains(alloc_range))
        {
          dbg.printf("Requested physical address range not covered by "
                     "factory constraints.\n");
          return -L4_ENOMEM;
        }

      ds_cont_regions.clear();
      static_cast<void>(
        ds_cont_regions.add(Moe::Mem_region::untyped(alloc_range)));
      // Fine to refer to stack-allocated list because on no-MMU all allocations
      // are continuous.
      ds_config.regions = {ds_cont_regions.begin(), ds_cont_regions.len()};
    }
  else if (!base.is_nil())
    return -L4_EINVAL;
#endif

  // L4::cout << "MEM: alloc ... " << size.value<l4_mword_t>()
  //          << "; " << flags.value<l4_umword_t>()
  //          << "; [" << L4::hex << mem_cfg.physmin
  //          << " .. " << mem_cfg.physmax << "]\n";
  cxx::unique_ptr<Moe::Dataspace> mo(alloc(size.value<l4_mword_t>(),
        ds_config, flags_val,
        align.is_of_int() ? align.value<l4_umword_t>() : 0));

  // L4::cout << "MO=" << mo.get() << "\n";
  res = register_ipc_object(cxx::move(mo), "moe-ds");
  // L4::cout << "MO_CAP=" << mo->obj_cap() << "\n";
  return L4_EOK;
}

l4_ret_t
Allocator::create_dma_space(L4::Ipc::Cap<void> &res)
{
  cxx::unique_ptr<Moe::Dma_space> o(make_obj<Moe::Dma_space>());
  res = register_ipc_object(cxx::move(o), "moe-dma-space");
  return L4_EOK;
}

l4_ret_t
Allocator::op_info(L4Re::Mem_alloc::Rights, L4Re::Mem_alloc::Stats &stats)
{
  Moe::Quota const *q = _qalloc.quota();

  stats.quota = q->limit();
  stats.quota_used = q->reserved();

  stats.mem_limit = cxx::min(stats.quota, Moe::Phys_limit::avail_ram);
  stats.mem_used = q->committed();

  // Clamp the reported amount of available memory to the factory quota limit
  // minus the already reserved amount. One cannot allocate more than the
  // remaining quota. Also, the globally remaining memory may be lower on
  // over-committed systems.
  size_t mem_avail = Single_page_alloc_base::_avail();
  stats.mem_free = cxx::min(stats.mem_limit - stats.mem_used,
                            stats.quota - stats.quota_used,
                            mem_avail);

  return L4_EOK;
}

#ifndef NDEBUG
l4_ret_t
Allocator::op_debug(L4Re::Debug_obj::Rights, unsigned long)
{
  Dbg out(Dbg::Info, "mem_alloc");
  if (_qalloc.quota()->limit() == (size_t)~0)
    out.printf("quota: no limit, reserved: %zu bytes (%zu MiB)\n",
               _qalloc.quota()->reserved(),
               _qalloc.quota()->reserved() / (1<<20));
  else
    out.printf("quota: limit: %zu bytes (%zu MiB), reserved: %zu bytes (%zu MiB), avail: %zu bytes (%zu MiB), committed: %zu bytes (%zu MiB)\n",
               _qalloc.quota()->limit(), _qalloc.quota()->limit() >> 20,
               _qalloc.quota()->reserved(), _qalloc.quota()->reserved() >> 20,
               _qalloc.quota()->limit() - _qalloc.quota()->reserved(),
               (_qalloc.quota()->limit() - _qalloc.quota()->reserved()) >> 20,
               _qalloc.quota()->committed(),
               _qalloc.quota()->committed() >> 20);
  out.printf("global: avail: %zu bytes (%zu MiB)\n",
             Single_page_alloc_base::_avail(),
             Single_page_alloc_base::_avail() / (1<<20));
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

  _root_alloc = Moe::Moe_alloc::allocator()->make_obj<Allocator>(
    Moe::Moe_alloc::allocator()->quota(), ~0, Moe::root_factory_config);
  object_pool.life.push_front(_root_alloc);
  return _root_alloc;
}
