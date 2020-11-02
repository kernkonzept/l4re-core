/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/bid_config.h>
#include <l4/cxx/minmax>

#include "dataspace_cont.h"
#include "pages.h"

Moe::Dataspace_cont::Dataspace_cont(void *start, unsigned long size,
                                    Flags flags,
                                    unsigned char page_shift)
: Dataspace(size, flags, page_shift), _start(static_cast<char*>(start))
{
  if (!can_cow())
    return;

  char *end = _start + l4_round_page(this->size());
  for (char *x = reinterpret_cast<char *>(
                   l4_trunc_page(reinterpret_cast<l4_addr_t>(_start)));
       x < end;
       x += L4_PAGESIZE)
    Moe::Pages::share(x);
}


Moe::Dataspace::Address
Moe::Dataspace_cont::address(l4_addr_t offset,
                             Flags flags, l4_addr_t hot_spot,
                             l4_addr_t min, l4_addr_t max) const
{
  if (!check_limit(offset))
    return Address(-L4_ERANGE);

  min = l4_trunc_page(min);
  //max = l4_round_page(max);

  l4_addr_t adr = l4_addr_t(_start) + offset;
  unsigned char order = L4_PAGESHIFT;

  while (order < 30 /* limit to 1GB flexpage */)
    {
      l4_addr_t map_base = l4_trunc_size(adr, order + 1);
      if (map_base < l4_addr_t(_start))
        break;

      if (map_base + (1UL << (order + 1)) -1 > (l4_addr_t(_start) + round_size() - 1))
        break;

      map_base = l4_trunc_size(hot_spot, order + 1);
      if (map_base < min)
        break;

      if (map_base + (1UL << (order + 1)) -1 > max)
        break;

      l4_addr_t mask = ~(~0UL << (order + 1));
      if (hot_spot == ~0UL || ((adr ^ hot_spot) & mask))
        break;

      ++order;
    }

  l4_addr_t map_base = l4_trunc_size(adr, order);
  l4_addr_t offs = adr & ~(~0UL << order);

  return Address(map_base, order, flags & map_flags(), offs);
}

int
Moe::Dataspace_cont::copy_address(l4_addr_t offset, Flags, l4_addr_t *addr,
                                  unsigned long *size) const
{
  if (!check_limit(offset))
    return -L4_ERANGE;

  *addr = l4_addr_t(_start) + offset;
  *size = this->size() - offset;
  return 0;
}

void Moe::Dataspace_cont::unmap() const noexcept
{
  unsigned long size = round_size();
  l4_addr_t offs = 0;
  enum { Fpages_array_length = L4_UTCB_GENERIC_DATA_SIZE - 2 };
  l4_fpage_t fpages[Fpages_array_length];
  unsigned pages_idx = 0;

  while (size)
    {
      Address addr = address(offs, L4Re::Dataspace::F::RWX);
      fpages[pages_idx++] = addr.fp();

      size -= (1UL << l4_fpage_size(addr.fp()));
      offs  += (1UL << l4_fpage_size(addr.fp()));

      // flush the fpages array, when it's full.
      if (pages_idx == Fpages_array_length)
        {
          l4_task_unmap_batch(L4_BASE_TASK_CAP, fpages, pages_idx,
                              L4_FP_OTHER_SPACES);
          pages_idx = 0;
        }
    }

  if (pages_idx > 0)
    l4_task_unmap_batch(L4_BASE_TASK_CAP, fpages, pages_idx,
                        L4_FP_OTHER_SPACES);
}

int
Moe::Dataspace_cont::dma_map(Dma_space * /* dma */, l4_addr_t offset,
                             l4_size_t *size,
                             Dma_attribs /* dma_attr */,
                             Dma_space::Direction /* dir */,
                             Dma_space::Dma_addr *dma_addr)
{
  if (offset >= this->size())
    return -L4_ERANGE;

  *dma_addr = reinterpret_cast<l4_addr_t>(start()) + offset;
  *size = cxx::min<l4_size_t>(*size, this->size() - offset);
  return 0;
}

#if !defined(CONFIG_MMU)
long
Moe::Dataspace_cont::map_info(l4_addr_t &start_addr,
                              l4_addr_t &end_addr) const noexcept
{
  start_addr = reinterpret_cast<l4_addr_t>(_start);
  end_addr = reinterpret_cast<l4_addr_t>(_start) + round_size() - 1U;
  return 1;
}
#endif
