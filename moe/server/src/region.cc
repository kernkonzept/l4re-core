/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "debug.h"
#include "region.h"
#include "dataspace_anon.h"
#include "dataspace_noncont.h"
#include "dataspace_util.h"

#include <l4/sys/kip>
#include <l4/sys/utcb.h>

#include <l4/re/rm>
#include <l4/re/util/cap>

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



int Region_handler::init(Region_map *rm, unsigned long size) noexcept
{
  if (!(_flags & (L4Re::Rm::F::Private | L4Re::Rm::F::Anonymous)))
    return L4_EOK;

  // In case of copy-on-write, the attached region must be fully backed by the
  // underlying dataspace.
  if (_mem)
    {
      _offs = l4_trunc_page(_offs);
      if (_offs >= _mem->size() || _mem->size() - _offs < size)
        return -L4_EINVAL;
    }

  _anon_mem = cxx::Ref_ptr<Moe::Dataspace>(
#ifdef CONFIG_MMU
    Moe::Dataspace_noncont::create(rm->qalloc(), size,
                                   Single_page_alloc_base::default_mem_cfg)
#else
    rm->qalloc()->make_obj<Moe::Dataspace_anon>(size, L4Re::Dataspace::F::RWX)
#endif
    );

  if (_mem)
    Dataspace_util::copy(_anon_mem.get(), 0, _mem.get(), _offs, size);

  _mem = _anon_mem.get();
  _offs = 0;

  return L4_EOK;
}

l4_ret_t Region_handler::map(l4_addr_t adr,
                             L4Re::Util::Region const &r, bool need_w,
                             Map_result *result) const noexcept
{
  if (!_mem)
    return -L4_EADDRNOTAVAIL;

  using L4::Ipc::Snd_fpage;
  l4_addr_t offs = adr - r.start();
  offs = l4_trunc_page(offs);
  auto f = map_flags(flags());
  if (!need_w)
    f -= L4Re::Dataspace::F::W;

  static Snd_fpage::Cacheopt const cache_map[] =
    { Snd_fpage::None, Snd_fpage::Buffered, Snd_fpage::Uncached,
      Snd_fpage::None };

  auto ds_fpage = _mem->address(offs + offset(), f, adr, r.start(), r.end());
  if (ds_fpage.is_nil())
    return -L4_EADDRNOTAVAIL;

  *result = Snd_fpage(ds_fpage.fp(), offs + r.start(), Snd_fpage::Map,
                      cache_map[caching() >> L4Re::Rm::Caching_shift]);

  return L4_EOK;
}

l4_ret_t Region_handler::page_in(L4Re::Util::Region const &r, l4_addr_t start,
                                 l4_addr_t end, L4Re::Rm::Region_flags rights,
                                 Map_result *result) const noexcept
{
  if (!_mem)
    return -L4_EADDRNOTAVAIL;

  using L4::Ipc::Snd_fpage;
  l4_addr_t offs = l4_trunc_page(start - r.start());

  static Snd_fpage::Cacheopt const cache_map[] =
    { Snd_fpage::None, Snd_fpage::Buffered, Snd_fpage::Uncached,
      Snd_fpage::None };

  L4Re::Rm::Region_flags rm_flags = (flags() & ~L4Re::Rm::F::Rights_mask)
                                  | rights;
  auto ds_fpage = _mem->address(offs + offset(), map_flags(rm_flags),
                                start, start, end);
  if (ds_fpage.is_nil())
    return -L4_EADDRNOTAVAIL;

  *result = Snd_fpage(ds_fpage.fp(), offs + r.start(), Snd_fpage::Map,
                      cache_map[caching() >> L4Re::Rm::Caching_shift]);

  return L4_EOK;
}

void
Region_handler::free(l4_addr_t start, unsigned long size) const noexcept
{
  if (!_mem)
    return;

  if (!(_flags & (L4Re::Rm::F::Private | L4Re::Rm::F::Anonymous)) && is_ro())
    return;

  _mem->clear(offset() + start, size);
}

l4_ret_t
Region_handler::map_info(l4_addr_t *start_addr, l4_addr_t *end_addr) const noexcept
{
  if (!_mem)
    return 0;

  if (flags() & (  L4Re::Rm::F::Pager
                 | L4Re::Rm::F::Reserved
                 | L4Re::Rm::F::Kernel))
    return 0;

  l4_ret_t ret = _mem->map_info(*start_addr, *end_addr);
  if (ret > 0)
    *start_addr += _offs;

  return ret;
}


l4_ret_t
Region_map::validate_ds(L4::Ipc::Snd_fpage const &ds_cap,
                        L4Re::Rm::Region_flags flags, Dataspace *ds)
{
  if (flags & L4Re::Rm::F::Pager)
    return -L4_EINVAL;

  if (!ds_cap.id_received())
    {
      // Release object reference to dataspace now. Otherwise, this has to wait
      // until the next received capability overwrites this dataspace capability
      // at the receive buffer.
      // This should/could be actually done by the RPC framework.
      L4Re::Util::cap_release(L4::Cap<void>(Rcv_cap << L4_CAP_SHIFT));
      return -L4_ENOENT;
    }

  auto *moe_ds = dynamic_cast<Moe::Dataspace*>(object_pool.find(ds_cap.data()));
  if (!moe_ds)
    return -L4_ENOENT;

  *ds = moe_ds;

  // If F::Private is set, a read-only DS can be mapped writable with CoW.
  auto ds_map_flags = moe_ds->map_flags(ds_cap.data());
  if (flags & L4Re::Rm::F::Private)
    ds_map_flags |= L4Re::Dataspace::F::W;

  if ((map_flags(flags) & ds_map_flags) != map_flags(flags))
    return -L4_EPERM;

  return L4_EOK;
}

l4_ret_t
Region_map::op_io_page_fault(L4::Io_pager::Rights,
                             l4_fpage_t io_pfa, l4_umword_t pc,
                             L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Dbg(Dbg::Warn).printf("IO-port-fault: port=0x%lx size=%d pc=0x%lx\n",
                        l4_fpage_ioport(io_pfa), 1 << l4_fpage_size(io_pfa), pc);
  return -L4_ENOMEM;
}

l4_ret_t
Region_map::op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                         L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  l4_addr_t pc = l4_utcb_exc_pc(&regs);
  if (l4_addr_t rescue = find_rescue_jump(pc))
    {
      l4_utcb_exc_pc_set(&regs, rescue);
      return L4_EOK;
    }

  Dbg(Dbg::Exceptions).printf("unhandled exception: PC=0x%lx PFA=0x%lx\n",
                              pc, l4_utcb_exc_pfa(&regs));
  return -L4_ENOREPLY;
}
