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

#include <stdio.h>

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
	  attach_area(start, end - start + 1, L4Re::Rm::F::Reserved);
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
  auto r_flags = h->flags();
  if (!writable)
    r_flags = r_flags & ~L4Re::Rm::F::W;

  if ((r_flags & Rm::F::Reserved) || !h->memory().is_valid())
    return -L4_ENOENT;

  if (r_flags & Rm::F::Pager)
    {
      L4::Ipc::Snd_fpage rfp;
      L4::cap_reinterpret_cast<L4::Pager>(h->memory())
        ->page_fault(local_addr, -3UL,
                     L4::Ipc::Rcv_fpage::mem(0, L4_WHOLE_ADDRESS_SPACE, 0),
                     rfp);
      return L4_EOK;
    }
  else
    {
      // align to 16byte, some DS implementations are too picky about
      // possible r/w etc. bits in the offset
      local_addr &= ~0x0fUL;
      l4_addr_t offset = local_addr - r.start() + h->offset();
      L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());
      L4Re::Dataspace::Flags flags = map_flags(r_flags);
      return ds->map(offset, flags, local_addr, r.start(), r.end());
    }
}

void
Region_ops::free(Region_handler const *h, l4_addr_t start, unsigned long size)
{
  if ((h->flags() & Rm::F::Reserved) || !h->memory().is_valid())
    return;

  if (h->flags() & Rm::F::Pager)
    return;

  L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());
  ds->clear(h->offset() + start, size);
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
    printf("  [%10lx-%10lx] -> (offs=%llx, ds=%lx, flags=%x)\n",
           i->first.start(), i->first.end(),
	   i->second.offset(), i->second.memory().cap(),
	   i->second.flags());
}

int
Region_map::op_exception(L4::Exception::Rights, l4_exc_regs_t &u,
                         L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Dbg w(Dbg::Warn);
  w.printf("%s: Unhandled exception: PC=0x%lx PFA=0x%lx LdrFlgs=0x%lx\n",
           Global::l4re_aux->binary, l4_utcb_exc_pc(&u), l4_utcb_exc_pfa(&u),
           Global::l4re_aux->ldr_flags);

  return -L4_ENOREPLY;
}

long
Region_map::op_io_page_fault(L4::Io_pager::Rights,
                             l4_fpage_t io_pfa, l4_umword_t pc,
                             L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Err().printf("IO-port-fault: port=0x%lx size=%d pc=0x%lx\n",
               l4_fpage_ioport(io_pfa), 1 << l4_fpage_size(io_pfa), pc);
  return -L4_ENOMEM;
}

