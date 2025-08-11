/**
 * \file
 * \brief  Region map client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/bid_config.h>
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
L4_RPC_DEF(L4Re::Rm::get_info);

namespace L4Re
{

long
Rm::attach(l4_addr_t *start, unsigned long size, Rm::Flags flags,
           L4::Ipc::Cap<Dataspace> mem, Rm::Offset offs,
           unsigned char align, L4::Cap<L4::Task> const task,
           char const *name, Rm::Offset backing_offset) const noexcept
{
  if ((flags & F::Rights_mask) == Flags(0)
      || (flags & (F::Reserved | F::Kernel)))
    mem = L4::Ipc::Cap<L4Re::Dataspace>();

  char const n = '\0';
  long e = attach_t::call(c(), start, size, flags, mem, offs, align,
                          mem.cap().cap(), name ? name : &n, backing_offset);
  if (e < 0)
    return e;

#ifdef CONFIG_MMU
  if ((flags & (F::Eager_map | F::No_eager_map)) == F::Eager_map)
#else
  if (!(flags & F::No_eager_map) && mem.is_valid())
#endif
    e = mem.cap()->map_region(offs, map_flags(flags), *start, *start + size,
                              task);

  return e;
}

int
Rm::detach(l4_addr_t start, unsigned long size, L4::Cap<Dataspace> *mem,
           L4::Cap<L4::Task> task, unsigned flags) const noexcept
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
