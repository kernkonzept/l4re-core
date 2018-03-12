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

#include <l4/sys/err.h>
#include <l4/re/rm>
#include <l4/re/env>
#include <l4/re/dataspace>
#include <l4/re/debug>

#include <l4/re/c/rm.h>
#include <l4/re/c/dataspace.h>

int
l4re_rm_reserve_area_srv(l4_cap_idx_t rm, l4_addr_t *start, unsigned long size,
                         unsigned flags, unsigned char align) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  return x->reserve_area(start, size, flags, align);
}

int
l4re_rm_free_area_srv(l4_cap_idx_t rm, l4_addr_t addr) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  return x->free_area(addr);
}

int
l4re_rm_attach_srv(l4_cap_idx_t rm, void **start, unsigned long size,
                   unsigned long flags, l4re_ds_t const mem, l4_addr_t offs,
                   unsigned char align) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  auto _mem = L4::Ipc::Cap<L4Re::Dataspace>::from_ci(mem);
  return x->attach(start, size, flags, _mem, offs, align);
}


int
l4re_rm_detach_srv(l4_cap_idx_t rm, l4_addr_t addr, l4re_ds_t *ds,
                   l4_cap_idx_t task) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  L4::Cap<L4::Task> t(task);
  L4::Cap<L4Re::Dataspace> d(L4_INVALID_CAP);
  int r = x->detach(addr, &d, t);
  if (ds)
    *ds = d.cap();
  return r;
}


int
l4re_rm_find_srv(l4_cap_idx_t rm, l4_addr_t *addr,
                 unsigned long *size, l4_addr_t *offset,
                 unsigned *flags, l4_cap_idx_t *m) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  L4::Cap<L4Re::Dataspace> mm(L4_INVALID_CAP);
  int r = x->find(addr, size, offset, flags, &mm);
  *m = mm.cap();
  return r;
}

void
l4re_rm_show_lists_srv(l4_cap_idx_t rm) L4_NOTHROW
{
  L4::Cap<L4Re::Debug_obj> d(rm);
  d->debug(0); // XXX: use enum
}
