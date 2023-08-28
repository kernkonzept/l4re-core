/**
 * \file
 * \brief  Memory allocator client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/mem_alloc>
#include <l4/re/mem_alloc-sys.h>
#include <l4/re/dataspace>
#include <l4/re/error_helper>

#include <l4/sys/factory>


namespace L4Re
{

l4_ret_t
Mem_alloc::alloc(long size,
                 L4::Cap<Dataspace> mem, unsigned long flags,
                 unsigned long align, l4_addr_t paddr) const noexcept
{
  L4::Cap<L4::Factory> f(cap());
  auto call = f->create(mem, L4Re::Dataspace::Protocol);
  call << l4_mword_t(size)
       << l4_umword_t(flags)
       << l4_umword_t(align);
  if (flags & Fixed_paddr)
    call << l4_umword_t(paddr);
  return l4_error(call);
}

};
