/**
 * \file
 * \brief  Dataspace client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/dataspace>
#include <l4/sys/cxx/ipc_client>
#include <l4/sys/cxx/consts>

L4_RPC_DEF(L4Re::Dataspace::clear);
L4_RPC_DEF(L4Re::Dataspace::allocate);
L4_RPC_DEF(L4Re::Dataspace::copy_in);
L4_RPC_DEF(L4Re::Dataspace::info);
L4_RPC_DEF(L4Re::Dataspace::map_info);

namespace L4Re {

long
Dataspace::__map(Dataspace::Offset offset, unsigned char *size,
                 Dataspace::Flags flags,
                 Dataspace::Map_addr local_addr,
                 L4::Cap<L4::Task> dst) const noexcept
{
  Map_addr spot = local_addr & ~(~0ULL << l4_umword_t(*size));
  Map_addr base = local_addr & (~0ULL << l4_umword_t(*size));
  L4::Ipc::Rcv_fpage r = L4::Ipc::Rcv_fpage::mem(base, *size, 0, dst);

  L4::Ipc::Snd_fpage fp;
  long err = map_t::call(c(), offset, spot, flags, r, fp, l4_utcb());
  if (L4_UNLIKELY(err < 0))
    return err;

  *size = fp.rcv_order();
  return err;
}

long
Dataspace::map_region(Dataspace::Offset offset, Dataspace::Flags flags,
                      Dataspace::Map_addr min_addr,
                      Dataspace::Map_addr max_addr,
                      L4::Cap<L4::Task> dst) const noexcept
{
  min_addr   = L4::trunc_page(min_addr);
  max_addr   = L4::round_page(max_addr);
  unsigned char order = L4_LOG2_PAGESIZE;

  long err = 0;

  while (min_addr < max_addr)
    {
      unsigned char order_mapped;
      order_mapped = order
        = L4::max_order(order, min_addr, min_addr, max_addr, min_addr);

      err = __map(offset, &order_mapped, flags, min_addr, dst);
      if (L4_UNLIKELY(err < 0))
        return err;

      if (order > order_mapped)
        order = order_mapped;

      min_addr += Map_addr(1) << order;
      offset   += Map_addr(1) << order;

      if (min_addr >= max_addr)
        return 0;

      while (min_addr != L4::trunc_order(min_addr, order)
             || max_addr < L4::round_order(min_addr + 1, order))
        --order;
    }

  return 0;
}

long
Dataspace::map(Dataspace::Offset offset, Dataspace::Flags flags,
               Dataspace::Map_addr local_addr,
               Dataspace::Map_addr min_addr,
               Dataspace::Map_addr max_addr,
               L4::Cap<L4::Task> dst) const noexcept
{
  min_addr   = L4::trunc_page(min_addr);
  max_addr   = L4::round_page(max_addr);
  local_addr = L4::trunc_page(local_addr);
  unsigned char order
    = L4::max_order(L4_LOG2_PAGESIZE, local_addr, min_addr, max_addr, local_addr);

  return __map(offset, &order, flags, local_addr, dst);
}

Dataspace::Size
Dataspace::size() const noexcept
{
  Stats stats = Stats();

  int err = info(&stats);
  if (err < 0)
    return 0;

  return stats.size;
}

Dataspace::Flags
Dataspace::flags() const noexcept
{
  Stats stats = Stats();

  int err = info(&stats);
  if (err < 0)
    return Flags(0);

  return stats.flags;
}

};
