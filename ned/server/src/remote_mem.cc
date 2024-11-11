/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "remote_mem.h"

#include <l4/bid_config.h>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/error_helper>

  void
Stack::set_stack(L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap const &ds, unsigned size)
{
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&_vma, size,
                                              L4Re::Rm::F::Search_addr | L4Re::Rm::F::RW,
                                              L4::Ipc::make_cap_rw(ds.get()),
                                              0),
               "attaching stack vma");
  _stack_ds = ds;
#ifdef CONFIG_MMU
  set_local_top(_vma.get() + size);
#else
  char *base = (char*)_vma.get();
  set_target_stack(l4_addr_t(base), size);
  set_local_top(base + size);
#endif
}
