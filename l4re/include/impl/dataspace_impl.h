/**
 * \file
 * \brief  Dataspace client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
#include <l4/re/dataspace>
#include <l4/sys/cxx/ipc_client>

L4_RPC_DEF(L4Re::Dataspace::clear);
L4_RPC_DEF(L4Re::Dataspace::allocate);
L4_RPC_DEF(L4Re::Dataspace::copy_in);
L4_RPC_DEF(L4Re::Dataspace::phys);
L4_RPC_DEF(L4Re::Dataspace::info);
L4_RPC_DEF(L4Re::Dataspace::take);
L4_RPC_DEF(L4Re::Dataspace::release);

namespace L4Re {

long
Dataspace::__map(unsigned long offset, unsigned char *size, unsigned long flags,
                 l4_addr_t local_addr) const throw()
{
  l4_addr_t spot = local_addr & ~(~0UL << l4_umword_t(*size));
  l4_addr_t base = local_addr & (~0UL << l4_umword_t(*size));
  L4::Ipc::Rcv_fpage r;
  r = L4::Ipc::Rcv_fpage::mem(base, *size, 0);

  L4::Ipc::Snd_fpage fp;
  long err = map_t::call(c(), offset, spot, flags, r, fp, l4_utcb());
  if (L4_UNLIKELY(err < 0))
    return err;

  *size = fp.rcv_order();
  return err;
}

long
Dataspace::map_region(l4_addr_t offset, unsigned long flags,
                      l4_addr_t min_addr, l4_addr_t max_addr) const throw()
{
  min_addr   = l4_trunc_page(min_addr);
  max_addr   = l4_round_page(max_addr);
  unsigned char order = L4_LOG2_PAGESIZE;

  long err = 0;

  while (min_addr < max_addr)
    {
      unsigned char order_mapped;
      order_mapped = order
        = l4_fpage_max_order(order, min_addr, min_addr, max_addr, min_addr);
      err = __map(offset, &order_mapped, flags, min_addr);
      if (L4_UNLIKELY(err < 0))
	return err;

      if (order > order_mapped)
	order = order_mapped;

      min_addr += 1UL << order;
      offset   += 1UL << order;

      if (min_addr >= max_addr)
	return 0;

      while (min_addr != l4_trunc_size(min_addr, order)
             || max_addr < l4_round_size(min_addr + 1,order))
	--order;
    }

  return 0;
}


long
Dataspace::map(l4_addr_t offset, unsigned long flags,
               l4_addr_t local_addr,
               l4_addr_t min_addr, l4_addr_t max_addr) const throw()
{
  min_addr   = l4_trunc_page(min_addr);
  max_addr   = l4_round_page(max_addr);
  local_addr = l4_trunc_page(local_addr);
  unsigned char order
    = l4_fpage_max_order(L4_LOG2_PAGESIZE, local_addr, min_addr, max_addr, local_addr);

  return __map(offset, &order, flags, local_addr);
}

long
Dataspace::size() const throw()
{
  Stats stats = Stats();
  int err = info(&stats);
  if (err < 0)
    return err;
  return stats.size;
}

long
Dataspace::flags() const throw()
{
  Stats stats = Stats();
  int err = info(&stats);
  if (err < 0)
    return err;
  return stats.flags;
}

};
