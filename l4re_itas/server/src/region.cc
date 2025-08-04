/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
  // Only get virtual address space limits. Reserved regions are gathered from
  // remote region manager and would collide otherwise.
  for (auto const &m: L4::Kip::Mem_desc::all(l4re_kip()))
    if (m.is_virtual() && m.type() == L4::Kip::Mem_desc::Conventional)
      set_limits(m.start(), m.end());
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
      return l4_error(L4::cap_reinterpret_cast<L4::Pager>(h->memory())
                      ->page_fault(local_addr, -3UL,
                                   L4::Ipc::Rcv_fpage::mem(0, L4_WHOLE_ADDRESS_SPACE),
                                   rfp));
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

int
Region_ops::map_info(Region_handler const *h,
                     l4_addr_t *start_addr, l4_addr_t *end_addr)
{
  if (!h->memory())
    return 0;

  if (h->flags() & (Rm::F::Pager | Rm::F::Reserved))
    return 0;

  return h->memory()->map_info(start_addr, end_addr);
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
    {
      unsigned f = i->second.flags();
      char r[8];
      r[0] = f & L4Re::Rm::F::R ? 'r' : '-';
      r[1] = f & L4Re::Rm::F::W ? 'w' : '-';
      r[2] = f & L4Re::Rm::F::X ? 'x' : '-';
      r[3] = f & L4Re::Rm::F::Kernel      ? 'K' : '-';
      r[4] = f & L4Re::Rm::F::Detach_free ? 'D' : '-';
      r[5] = f & L4Re::Rm::F::Pager       ? 'P' : '-';
      r[6] = f & L4Re::Rm::F::Reserved    ? 'R' : '-';
      r[7] = '\0';
      printf("  %010lx-%010lx ds=%04lx@%07llx %s/%04x %07llx: %.*s \n",
             i->first.start(), i->first.end(),
             i->second.memory().cap() >> L4_CAP_SHIFT, i->second.offset(),
             r, i->second.flags(),
             i->first.backing_offset(), i->first.name_len(), i->first.name());
    }
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

