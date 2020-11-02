/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace.h"
#include "dataspace_util.h"
#include "globals.h"
#include "page_alloc.h"

#include <l4/bid_config.h>
#include <l4/re/util/meta>

#include <l4/cxx/iostream>
#include <l4/cxx/minmax>

#include <l4/sys/capability>
#include <l4/sys/err.h>
#include <l4/sys/cache.h>

#include <cstring>
using cxx::min;

int
Moe::Dataspace::map(l4_addr_t offs, l4_addr_t hot_spot, Flags flags,
                    l4_addr_t min, l4_addr_t max, L4::Ipc::Snd_fpage &memory)
{
  using L4Re::Dataspace;
  using L4::Ipc::Snd_fpage;

  memory = L4::Ipc::Snd_fpage();

  offs     = l4_trunc_page(offs);
  hot_spot = l4_trunc_page(hot_spot);

  if (!check_limit(offs))
    {
      if (1)
        L4::cout << "MOE: ds access out of bounds: offset=" << L4::n_hex(offs)
                 << " size=" << L4::n_hex(size()) << "\n";

      return -L4_ERANGE;
    }

  Address adr = address(offs, flags, hot_spot, min, max);
  if (adr.is_nil())
    return adr.error();

  unsigned long cache_opt = cxx::max((_flags & Dataspace::F::Caching_mask).raw,
                                     (flags & Dataspace::F::Caching_mask).raw);

  static Snd_fpage::Cacheopt cache_map[] =
    { Snd_fpage::None, Snd_fpage::Buffered, Snd_fpage::Uncached,
      Snd_fpage::None };

  memory = Snd_fpage(adr.fp(), hot_spot, Snd_fpage::Map,
                     cache_map[cache_opt >> Dataspace::F::Caching_shift]);

  return L4_EOK;
}

long
Moe::Dataspace::op_map(L4Re::Dataspace::Rights rights,
                       L4Re::Dataspace::Offset offset,
                       L4Re::Dataspace::Map_addr spot,
                       L4Re::Dataspace::Flags flags,
                       L4::Ipc::Snd_fpage &fp)
{
  auto mf = map_flags(rights);

  if (0)
    L4::cout << "MAPrq: " << L4::hex << offset << ", " << spot << ", "
      << flags.raw << "\n";

  if (!mf.w() && flags.w())
    return -L4_EPERM;

  if (!mf.x() && flags.x())
    return -L4_EPERM;

  long ret = map(offset, spot, flags & mf, 0, ~0, fp);

  if (0)
    L4::cout << "MAP: " << L4::hex << reinterpret_cast<unsigned long *>(&fp)[0]
             << ", " << reinterpret_cast<unsigned long *>(&fp)[1]
             << ", " << flags.raw << ", " << (mf.w() && flags.w())
             << ", ret=" << ret << '\n';

  return ret;
};

long
Moe::Dataspace::op_copy_in(L4Re::Dataspace::Rights obj,
                           L4Re::Dataspace::Offset dst_offs,
                           L4::Ipc::Snd_fpage const &src_cap,
                           L4Re::Dataspace::Offset src_offs,
                           L4Re::Dataspace::Size sz)
{
  Moe::Dataspace *src = 0;

  if (src_cap.id_received())
    src = dynamic_cast<Moe::Dataspace*>(object_pool.find(src_cap.data()));

  if (!map_flags(obj).w())
    return -L4_EACCESS;

  if (!src)
    return -L4_EINVAL;

  if (sz == 0)
    return L4_EOK;

  Dataspace_util::copy(this, dst_offs, src, src_offs, sz);

  return L4_EOK;
}

long
Moe::Dataspace::map_info(l4_addr_t &, l4_addr_t &) const noexcept
{
#ifdef CONFIG_MMU
  return 0;
#else
  return -L4_EPERM;
#endif
}

long
Moe::Dataspace::op_map_info(L4Re::Dataspace::Rights /*rights*/,
                            l4_addr_t &start_addr,
                            l4_addr_t &end_addr)
{ return map_info(start_addr, end_addr); }

long
Moe::Dataspace::clear(l4_addr_t offs, unsigned long size) const noexcept
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = min(size, round_size() - offs);

  while (sz)
    {
      Address dst_a = address(offs);
      unsigned long b_sz = min(dst_a.sz() - dst_a.of(), sz);

      memset(dst_a.adr(), 0, b_sz);

      // No need for I cache coherence, as we just zero fill and assume that
      // this is no executable code
      l4_cache_clean_data(reinterpret_cast<l4_addr_t>(dst_a.adr()),
                          reinterpret_cast<l4_addr_t>(dst_a.adr()) + b_sz);

      offs += b_sz;
      sz -= b_sz;
    }

  return 0;
}

int
Moe::Dataspace::dma_map(Dma_space *, l4_addr_t, l4_size_t *,
                        Dma_attribs, Dma_space::Direction,
                        Dma_space::Dma_addr *)
{
  return -L4_EINVAL;
}
