/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "debug.h"
#include "region.h"

#include <l4/sys/kip>

#include <l4/re/rm>

#include <l4/cxx/iostream>
#include <l4/cxx/exceptions>

Region_map::Region_map()
  : Base(Moe::Virt_limit::start, Moe::Virt_limit::end)
{
  for (auto const &m: L4::Kip::Mem_desc::all(kip()))
    {
      if (m.type() != L4::Kip::Mem_desc::Reserved || !m.is_virtual())
        continue;

      l4_addr_t start = m.start();
      l4_addr_t end = m.end();

      attach_area(start, end - start + 1, L4Re::Rm::F::Reserved);
    }

#ifdef CONFIG_MMU
  // Prevent NULL pointer accesses on MMU systems. On systems without MMU there
  // might actually be valid memory on this address.
  attach_area(0, L4_PAGESIZE);
#endif
}

int Region_ops::map(Region_handler const *h, l4_addr_t adr,
                    L4Re::Util::Region const &r, bool need_w,
                    L4::Ipc::Snd_fpage *result)
{
  if (!h->memory())
    return -L4_EADDRNOTAVAIL;

  using L4::Ipc::Snd_fpage;
  l4_addr_t offs = adr - r.start();
  offs = l4_trunc_page(offs);
  auto f = map_flags(h->flags());
  if (!need_w)
    f &= ~L4Re::Dataspace::F::W;

  static Snd_fpage::Cacheopt const cache_map[] =
    { Snd_fpage::None, Snd_fpage::Buffered, Snd_fpage::Uncached,
      Snd_fpage::None };

  auto ds_fpage = h->memory()->address(offs + h->offset(), f, adr,
                                       r.start(), r.end());
  if (ds_fpage.is_nil())
    return -L4_EADDRNOTAVAIL;

  *result = Snd_fpage(ds_fpage.fp(), offs + r.start(), Snd_fpage::Map,
                      cache_map[h->caching() >> L4Re::Rm::Caching_shift]);

  return L4_EOK;
}

void
Region_ops::free(Region_handler const *h, l4_addr_t start, unsigned long size)
{
  if (h->is_ro() || !h->memory())
    return;

  h->memory()->clear(h->offset() + start, size);
}

int
Region_ops::map_info(Region_handler const *h,
                     l4_addr_t *start_addr, l4_addr_t *end_addr)
{
  if (!h->memory())
    return 0;

  if (h->flags() & (  L4Re::Rm::F::Pager
                    | L4Re::Rm::F::Reserved
                    | L4Re::Rm::F::Kernel))
    return 0;

  return h->memory()->map_info(*start_addr, *end_addr);
}


int
Region_map::validate_ds(void *, L4::Ipc::Snd_fpage const &ds_cap,
                        L4Re::Rm::Region_flags flags, Dataspace *ds)
{
  if (flags & L4Re::Rm::F::Pager)
    return -L4_EINVAL;

  if (!ds_cap.id_received())
    return -L4_ENOENT;

  auto *moe_ds = dynamic_cast<Moe::Dataspace*>(object_pool.find(ds_cap.data()));

  if (!moe_ds)
    return -L4_ENOENT;

  *ds = moe_ds;

  if ((map_flags(flags) & moe_ds->map_flags(ds_cap.data())) != map_flags(flags))
    return -L4_EPERM;

  return L4_EOK;
}

long
Region_map::op_io_page_fault(L4::Io_pager::Rights,
                             l4_fpage_t io_pfa, l4_umword_t pc,
                             L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Dbg(Dbg::Warn).printf("IO-port-fault: port=0x%lx size=%d pc=0x%lx\n",
                        l4_fpage_ioport(io_pfa), 1 << l4_fpage_size(io_pfa), pc);
  return -L4_ENOMEM;
}
