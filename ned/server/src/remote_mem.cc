/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "remote_mem.h"

#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/error_helper>

  void
Stack::set_stack(L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap const &ds, unsigned size)
{
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&_vma, size,
                                              L4Re::Rm::Search_addr,
                                              L4::Ipc::make_cap_rw(ds.get()),
                                              0),
               "attaching stack vma");
  _stack_ds = ds;
  set_local_top((char*)(_vma.get() + size));
}

