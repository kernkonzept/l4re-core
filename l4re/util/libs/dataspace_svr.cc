// vi:ft=cpp
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <l4/sys/types.h>
#include <l4/cxx/list>
#include <l4/cxx/minmax>
#include <l4/re/dataspace>
#include <l4/re/dataspace-sys.h>
#include <l4/re/util/dataspace_svr>

#if 0
inline
L4::Ipc::Ostream &operator << (L4::Ipc_ostream &s,
                              L4Re::Dataspace::Stats const &st)
{ s.put(st); return s; }
#endif

namespace L4Re { namespace Util {

int
Dataspace_svr::map(l4_addr_t offs, l4_addr_t hot_spot, unsigned long flags,
                    l4_addr_t min, l4_addr_t max, L4::Ipc::Snd_fpage &memory)
{
  int err = map_hook(offs, flags, min, max);
  if (err < 0)
    return err;

  memory = L4::Ipc::Snd_fpage();

  offs     = l4_trunc_page(offs);
  hot_spot = l4_trunc_page(hot_spot);

  if (!check_limit(offs))
    {
#if 0
      printf("limit failed: off=%lx sz=%lx\n", offs, size());
#endif
      return -L4_ERANGE;
    }

  min = l4_trunc_page(min);
  max = l4_round_page(max);

  l4_addr_t addr = _ds_start + offs;
  unsigned char order = L4_PAGESHIFT;

  while (order < 30 /* limit to 1GB flexpage */)
    {
      l4_addr_t map_base = l4_trunc_size(addr, order + 1);
      if (map_base < _ds_start)
	break;

      if (map_base + (1UL << (order + 1)) -1 > (_ds_start + round_size() - 1))
	break;

      map_base = l4_trunc_size(hot_spot, order + 1);
      if (map_base < min)
	break;

      if (map_base + (1UL << (order + 1)) -1 > max -1)
	break;

      l4_addr_t mask = ~(~0UL << (order + 1));
      if (hot_spot == ~0UL || ((addr ^ hot_spot) & mask))
	break;

      ++order;
    }

  l4_addr_t map_base = l4_trunc_size(addr, order);
  //l4_addr_t map_offs = addr & ~(~0UL << order);

  l4_fpage_t fpage = l4_fpage(map_base, order, flags && is_writable() ?  L4_FPAGE_RWX : L4_FPAGE_RX);

  memory = L4::Ipc::Snd_fpage(fpage, hot_spot, _map_flags, _cache_flags);

  return L4_EOK;
}

long
Dataspace_svr::clear(l4_addr_t offs, unsigned long ds_size) const throw()
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = ds_size = cxx::min(ds_size, round_size() - offs);

  while (sz)
    {
      unsigned long b_addr = _ds_start + offs;
      unsigned long b_sz = cxx::min(ds_size - offs, sz);

      memset((void *)b_addr, 0, b_sz);

      offs += b_sz;
      sz -= b_sz;
    }

  return ds_size;
}

int
Dataspace_svr::phys(l4_addr_t /*offset*/, l4_addr_t &/*phys_addr*/, l4_size_t &/*phys_size*/) throw()
{
  return -L4_EINVAL;
}

}}
