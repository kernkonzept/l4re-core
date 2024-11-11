/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/c/mem_alloc.h>

#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/env>

#include <l4/sys/err.h>


long
l4re_ma_alloc_align_srv(l4_cap_idx_t srv, long size,
                        l4re_ds_t const mem, unsigned long flags,
                        unsigned long align) L4_NOTHROW
{
  L4::Cap<L4Re::Mem_alloc> x(srv);
  L4::Cap<L4Re::Dataspace> ds(mem);
  return x->alloc(size, ds, flags, align);
}
