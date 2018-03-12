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

#include <l4/re/util/meta>

#include <l4/cxx/iostream>
#include <l4/cxx/minmax>

#include <l4/sys/capability>
#include <l4/sys/err.h>

#include <cstring>
using cxx::min;

int
Moe::Dataspace::map(l4_addr_t offs, l4_addr_t hot_spot, unsigned long flags,
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

  Ds_rw rw = (flags & Writable) ? Writable : Read_only;
  Address adr = address(offs, rw, hot_spot, min, max);
  if (adr.is_nil())
    return -L4_EPERM;

  unsigned long cache_opt = cxx::max(_flags & Dataspace::Map_caching_mask,
                                     (unsigned short)flags & Dataspace::Map_caching_mask);

  static Snd_fpage::Cacheopt cache_map[] =
    { Snd_fpage::None, Snd_fpage::Buffered, Snd_fpage::Uncached };

  memory = Snd_fpage(adr.fp(), hot_spot, Snd_fpage::Map,
                     cache_map[cache_opt >> Dataspace::Map_caching_shift]);

  return L4_EOK;
}

long
Moe::Dataspace::op_map(L4Re::Dataspace::Rights obj,
                       unsigned long offset, l4_addr_t spot,
                       unsigned long flags, L4::Ipc::Snd_fpage &fp)
{
  bool read_only = !is_writable() || !(obj & L4_CAP_FPAGE_W);

  if (0)
    L4::cout << "MAPrq: " << L4::hex << offset << ", " << spot << ", "
      << flags << "\n";

  if (read_only && (flags & Writable))
    return -L4_EPERM;

  long ret = map(offset, spot, flags, 0, ~0, fp);

  if (0)
    L4::cout << "MAP: " << L4::hex << reinterpret_cast<unsigned long *>(&fp)[0]
             << ", " << reinterpret_cast<unsigned long *>(&fp)[1]
             << ", " << flags << ", " << (!read_only && (flags & Writable))
             << ", ret=" << ret << '\n';

  return ret;
};

long
Moe::Dataspace::op_copy_in(L4Re::Dataspace::Rights obj,
                           l4_addr_t dst_offs, L4::Ipc::Snd_fpage const &src_cap,
                           l4_addr_t src_offs, unsigned long sz)
{
  Moe::Dataspace *src = 0;

  if (src_cap.id_received())
    src = dynamic_cast<Moe::Dataspace*>(object_pool.find(src_cap.data()));

  if (!(obj & L4_CAP_FPAGE_W))
    return -L4_EACCESS;

  if (!src)
    return -L4_EINVAL;

  if (sz == 0)
    return L4_EOK;

  Dataspace_util::copy(this, dst_offs, src, src_offs, sz);

  return L4_EOK;
}

long
Moe::Dataspace::clear(l4_addr_t offs, unsigned long _size) const throw()
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = _size = min(_size, round_size()-offs);

  while (sz)
    {
      Address dst_a = address(offs, Writable);
      unsigned long b_sz = min(dst_a.sz() - dst_a.of(), sz);

      memset(dst_a.adr(), 0, b_sz);

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

int
Moe::Dataspace::dma_unmap(Dma_space *, l4_addr_t, l4_size_t,
                          Dma_attribs, Dma_space::Direction)
{
  return -L4_EINVAL;
}
