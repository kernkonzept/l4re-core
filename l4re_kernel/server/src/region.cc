/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "region.h"
#include "globals.h"

#include <l4/sys/kip>

#include <l4/re/dataspace>
#include <l4/re/rm>
#include <l4/re/env>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include "dispatcher.h"
#include "debug.h"

#include <cstdio>

using L4Re::Rm;
using L4Re::Dataspace;
using L4Re::Util::Region;

Region_map::Region_map()
  : Base(0,0)
{}

void
Region_map::init()
{
  extern char __L4_KIP_ADDR__[];

  for (auto const &m: L4::Kip::Mem_desc::all(__L4_KIP_ADDR__))
    {
      if (!m.is_virtual())
	continue;

      l4_addr_t start = m.start();
      l4_addr_t end = m.end();
      
      switch (m.type())
	{
	case L4::Kip::Mem_desc::Conventional:
	  set_limits(start, end);
	  break;
	case L4::Kip::Mem_desc::Reserved:
	  attach_area(start, end - start + 1, L4Re::Rm::Reserved);
	  break;
	default:
	  break;
	}
    }

  // reserve page at 0
  attach_area(0, L4_PAGESIZE);
}


int
Region_ops::map(Region_handler const *h, l4_addr_t local_addr,
                Region const &r, bool writable, l4_umword_t *result)
{
  *result = 0;
  if ((h->flags() & Rm::Reserved) || !h->memory().is_valid())
    return -L4_ENOENT;

  if (h->flags() & Rm::Pager)
    {
      l4_mword_t result;
      L4::Ipc::Snd_fpage rfp;
      L4::cap_reinterpret_cast<L4::Pager>(h->memory())
        ->page_fault((local_addr | (writable ? 2 : 0)), -3UL, result,
                     L4::Ipc::Rcv_fpage::mem(0, L4_WHOLE_ADDRESS_SPACE, 0),
                     rfp);
      return L4_EOK;
    }
  else
    {
      l4_addr_t offset = local_addr - r.start() + h->offset();
      L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());
      unsigned flags = writable | (h->caching() >> Rm::Caching_ds_shift);
      return ds->map(offset, flags, local_addr, r.start(), r.end());
    }
}

void Region_ops::unmap(Region_handler const *h, l4_addr_t vaddr,
                       l4_addr_t offs, unsigned long size)
{
  (void)h; (void)vaddr; (void)offs; (void)size;
#if 0
  for (l4_addr_t a = vaddr; a < vaddr + size; a += L4_PAGESIZE)
    l4_task_unmap(L4Re::This_task,
                  l4_fpage(a, L4_LOG2_PAGESIZE, L4_FPAGE_RWX),
                  L4_FP_ALL_SPACES);

  if (h->flags() & (Rm::Pager | Rm::Reserved))
    return;

  if (!(h->flags() & Rm::No_alias))
    return;

  L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());

  // swipe out memory pages if they are not aliasd somewhere else
  // the DSM may free thoses pages
  ds->clear(offs, size);
#endif
}

void
Region_ops::free(Region_handler const *h, l4_addr_t start, unsigned long size)
{
  if ((h->flags() & Rm::Reserved) || !h->memory().is_valid())
    return;

  if (h->flags() & Rm::Pager)
    return;

  L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());
  ds->clear(h->offset() + start, size);
}

void
Region_ops::take(Region_handler const * /*h*/)
{
#if 0
  if (h->flags() & (Rm::Pager | Rm::Reserved))
    return;

  L4::cap_cast<L4Re::Dataspace>(h->memory())->take();
#endif
}

void
Region_ops::release(Region_handler const * /*h*/)
{
#if 0
  if (h->flags() & (Rm::Pager | Rm::Reserved))
    return;

  L4::cap_cast<L4Re::Dataspace>(h->memory())->release();
#endif
}

void
Region_map::debug_dump(unsigned long /*function*/) const
{
  printf("Region mapping: limits [%lx-%lx]\n", min_addr(), max_addr());
  printf(" Area map:\n");
  for (Region_map::Const_iterator i = area_begin(); i != area_end(); ++i)
    printf("  [%10lx-%10lx] -> flags=%x\n",
           i->first.start(), i->first.end(),
	   i->second.flags());
  printf(" Region map:\n");
  for (Region_map::Const_iterator i = begin(); i != end(); ++i)
    printf("  [%10lx-%10lx] -> (offs=%lx, ds=%lx, flags=%x)\n",
           i->first.start(), i->first.end(),
	   i->second.offset(), i->second.memory().cap(),
	   i->second.flags());
}

int
Region_map::op_exception(L4::Exception::Rights, l4_exc_regs_t &u,
                         L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Dbg w(Dbg::Warn);
  w.printf("unhandled exception: pc=0x%lx (pfa=%lx)\n", l4_utcb_exc_pc(&u),
           l4_utcb_exc_pfa(&u));
  w.printf("Global::l4re_aux->ldr_flags=%lx\n", Global::l4re_aux->ldr_flags);

  return -L4_ENOREPLY;
}

long
Region_map::op_io_page_fault(L4::Io_pager::Rights,
                             l4_fpage_t io_pfa, l4_umword_t pc,
                             L4::Ipc::Opt<l4_mword_t> &result,
                             L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Err().printf("IO-port-fault: port=0x%lx size=%d pc=0x%lx\n",
               l4_fpage_ioport(io_pfa), 1 << l4_fpage_size(io_pfa), pc);
  result = ~0;
  return -1;
}

