/**
 * \file
 * \brief  Region map client stub implementation
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
#include <l4/re/rm>
#include <l4/re/dataspace>

#include <l4/sys/cxx/ipc_client>

#include <l4/sys/task>
#include <l4/sys/err.h>

L4_RPC_DEF(L4Re::Rm::reserve_area);
L4_RPC_DEF(L4Re::Rm::free_area);
L4_RPC_DEF(L4Re::Rm::attach);
L4_RPC_DEF(L4Re::Rm::detach);
L4_RPC_DEF(L4Re::Rm::get_regions);
L4_RPC_DEF(L4Re::Rm::get_areas);
L4_RPC_DEF(L4Re::Rm::find);

namespace L4Re
{

long
Rm::attach(l4_addr_t *start, unsigned long size, unsigned long flags,
           L4::Ipc::Cap<Dataspace> mem, l4_addr_t offs,
           unsigned char align) const throw()
{
  if (flags & Reserved)
    mem = L4::Ipc::Cap<L4Re::Dataspace>();

  long e = attach_t::call(c(), start, size, flags, mem, offs, align, mem.cap().cap());
  if (e < 0)
    return e;

  if (flags & Eager_map)
    {
      unsigned long fl = (flags & Read_only)
        ? Dataspace::Map_ro
        : Dataspace::Map_rw;
      fl |= (flags & Caching) >> Caching_ds_shift;
      e = mem.cap()->map_region(offs, fl, *start, *start + size);
    }
  return e;
}

int
Rm::detach(l4_addr_t start, unsigned long size, L4::Cap<Dataspace> *mem,
           L4::Cap<L4::Task> task, unsigned flags) const throw()
{
  l4_addr_t rstart = 0, rsize = 0;
  l4_cap_idx_t mem_cap = L4_INVALID_CAP;
  long e = detach_t::call(c(), start, size, flags, rstart, rsize, mem_cap);
  if (L4_UNLIKELY(e < 0))
    return e;

  if (mem)
    *mem = L4::Cap<L4Re::Dataspace>(mem_cap);

  if (!task.is_valid())
    return e;

  rsize = l4_round_page(rsize);
  unsigned order = L4_LOG2_PAGESIZE;
  unsigned long sz = (1UL << order);
  for (unsigned long p = rstart; rsize; p += sz, rsize -= sz)
    {
      while (sz > rsize)
        {
          --order;
          sz >>= 1;
        }

      for (;;)
        {
          unsigned long m = sz << 1;
          if (m > rsize)
            break;

          if (p & (m - 1))
            break;

          ++order;
          sz <<= 1;
        }

      task->unmap(l4_fpage(p, order, L4_FPAGE_RWX),
                  L4_FP_ALL_SPACES);
    }

  return e;
}
}
