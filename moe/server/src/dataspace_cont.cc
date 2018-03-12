/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/minmax>

#include "dataspace_cont.h"
#include "pages.h"

Moe::Dataspace_cont::Dataspace_cont(void *start, unsigned long size,
                                    unsigned short flags,
                                    unsigned char page_shift)
: Dataspace(size, flags, page_shift), _start((char*)start)
{
  if (!can_cow())
    return;

  char *end = _start + l4_round_page(this->size());
  for (char *x = (char *)l4_trunc_page((l4_addr_t)_start); x < end;
       x += L4_PAGESIZE)
    Moe::Pages::share(x);
}


Moe::Dataspace::Address 
Moe::Dataspace_cont::address(l4_addr_t offset,
                             Ds_rw rw, l4_addr_t hot_spot,
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

  if (!is_writable())
    rw = Read_only;

  return Address(map_base, order, rw, offs);
}

void Moe::Dataspace_cont::unmap(bool ro) const throw()
{
  unsigned long _size = round_size();
  l4_addr_t offs = 0;

  while (_size)
    {
      Address addr = address(offs, Writable, ~0);
      l4_fpage_t fp
        = l4_fpage_set_rights(addr.fp(), ro ? L4_FPAGE_W : L4_FPAGE_RWX);
      l4_task_unmap(L4_BASE_TASK_CAP, fp, L4_FP_OTHER_SPACES);
      _size -= (1UL << l4_fpage_size(fp));
      offs  += (1UL << l4_fpage_size(fp));
    }
}

int
Moe::Dataspace_cont::dma_map(Dma_space *dma, l4_addr_t offset, l4_size_t *size,
                             Dma_attribs dma_attr, Dma_space::Direction dir,
                             Dma_space::Dma_addr *dma_addr)
{
  (void)dma;
  (void)dma_attr;
  (void)dir;

  if (offset >= this->size())
    return -L4_ERANGE;

  *dma_addr = (l4_addr_t)start() + offset;
  *size = cxx::min(*size, (l4_size_t)(this->size() - offset));
  return 0;
}

int
Moe::Dataspace_cont::dma_unmap(Dma_space *, l4_addr_t, l4_size_t,
                               Dma_attribs, Dma_space::Direction)
{
  return 0;
}
