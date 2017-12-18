/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "app_model.h"

#include <l4/re/error_helper>
#include <l4/re/util/env_ns>

using L4Re::chksys;
using L4Re::chkcap;

App_model::Dataspace
App_model::alloc_ds(unsigned long size) const
{
  Dataspace mem = chkcap(L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>(),
                         "allocate capability");
  L4::Cap<L4Re::Mem_alloc> _ma(prog_info()->mem_alloc.raw & L4_FPAGE_ADDR_MASK); 
  chksys(_ma->alloc(size, mem.get()), "allocate writable program segment");
  return mem;
}

App_model::Const_dataspace
App_model::open_file(char const *name)
{
  L4Re::Util::Env_ns ens;
  return L4Re::chkcap(ens.query<L4Re::Dataspace>(name), name, 0);
}

void
App_model::prog_attach_ds(l4_addr_t addr, unsigned long size,
                          Const_dataspace ds, unsigned long offset,
                          unsigned flags, char const *what)
{
  unsigned rh_flags = flags;

  if (0)
    printf("%s:%s: from ds:%lx+%lx... @%lx+%lx\n",
           __func__, what, ds.cap(), offset, addr, size);

  if (!ds.is_valid())
    rh_flags |= L4Re::Rm::Reserved;

  l4_addr_t _addr = addr;
  L4Re::chksys(_task->rm()->attach(&_addr, size, rh_flags,
                                   L4::Ipc::make_cap(ds.get(), (flags & L4Re::Rm::Read_only)
                                                               ? L4_CAP_FPAGE_RO
                                                               : L4_CAP_FPAGE_RW),
                                   offset, 0), what);
}

int
App_model::prog_reserve_area(l4_addr_t *start, unsigned long size,
                             unsigned flags, unsigned char align)
{
  return _task->rm()->reserve_area(start, size, flags, align);
}


void
App_model::copy_ds(Dataspace dst, unsigned long dst_offs,
                   Const_dataspace src, unsigned long src_offs,
                   unsigned long size)
{
  L4Re::chksys(dst->copy_in(dst_offs, src.get(), src_offs, size),
               "Ned program launch: copy failed");
}


l4_addr_t
App_model::local_attach_ds(Const_dataspace ds, unsigned long size,
                           unsigned long offset) const
{
  L4::Cap<L4Re::Rm> rm = L4Re::Env::env()->rm();
  l4_addr_t pg_offset = l4_trunc_page(offset);
  l4_addr_t in_pg_offset = offset - pg_offset;
  unsigned long pg_size = l4_round_page(size + in_pg_offset);
  l4_addr_t vaddr = 0;
  chksys(rm->attach(&vaddr, pg_size,
	L4Re::Rm::Search_addr | L4Re::Rm::Read_only,
	ds.get(), pg_offset), "attach temporary VMA");
  return vaddr + in_pg_offset;
}

void
App_model::local_detach_ds(l4_addr_t addr, unsigned long /*size*/) const
{
  L4::Cap<L4Re::Rm> rm = L4Re::Env::env()->rm();
  l4_addr_t pg_addr = l4_trunc_page(addr);
  chksys(rm->detach(pg_addr, 0), "detach temporary VMA");
}


App_model::App_model()
: _task(0)
{
  // set default values for utcb area, values may be changed by loader
  _info.utcbs_start     = Utcb_area_start;
  _info.utcbs_log2size  = L4_PAGESHIFT;

  // set default values for the application stack
  extern char __L4_KIP_ADDR__[];
  _info.kip = (l4_addr_t)__L4_KIP_ADDR__;
}


App_model::Dataspace
App_model::alloc_app_stack()
{
  L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap stack
    = chkcap(L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>(),
             "allocate stack capability");
  L4::Cap<L4Re::Mem_alloc> ma(prog_info()->mem_alloc.raw & L4_FPAGE_ADDR_MASK);
  chksys(ma->alloc(_stack.stack_size(), stack.get()),
         "allocate stack");

  _stack.set_stack(stack, _stack.stack_size());

  return stack.release();
}

void
App_model::init_prog()
{

  push_argv_strings();
  push_env_strings();
  Dbg info(Dbg::Info);
#if 0
  _info.mem_alloc = _ma.fpage();
  _info.names = _ns.fpage();
  _info.log = _log.fpage();
  _info.factory = _factory.fpage();
  _info.scheduler = _sched.fpage();
  _info.l4re_dbg = ~0;
#endif
#if 0
  _info.ldr_flags = parser.ldr_flags.data;
  _info.l4re_dbg = parser.l4re_dbg.data;

  if (parser.cpu_affinity.data != ~0UL || parser.base_prio.data != Default_base_prio
      || parser.max_prio.data != Default_max_prio)
    {
      info.printf("    base_prio = 0x%x max_prio = 0x%x\n",
                  parser.base_prio.data, parser.max_prio.data);
      _task->_sched.set_prio(parser.base_prio.data, parser.max_prio.data);
      _task->_sched.restrict_cpus(parser.cpu_affinity.data);
      Gate_alloc::alloc(&_task->_sched);
      _info.scheduler = _task->_sched.obj_cap().fpage();
      scheduler_if = &_task->_sched;
    }
#endif
  info.printf("loading '%s'\n", argv.a0);
}

void
App_model::get_task_caps(L4::Cap<L4::Factory> *factory,
                             L4::Cap<L4::Task> *task,
                             L4::Cap<L4::Thread> *thread)
{
  prog_info()->rm = _task->rm().fpage();
  prog_info()->parent = _task->obj_cap().fpage();
  Dbg(Dbg::Info).printf("parent cap is %lx\n", prog_info()->parent.raw);
  *task = _task->task_cap();
  *thread = _task->thread_cap();
  *factory = L4::Cap<L4::Factory>(prog_info()->factory.raw & L4_FPAGE_ADDR_MASK);
}

