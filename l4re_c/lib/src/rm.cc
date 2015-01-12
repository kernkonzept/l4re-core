/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
                         l4re_rm_flags_t flags, unsigned char align) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  return x->reserve_area(start, size, L4Re::Rm::Flags(flags), align);
}

int
l4re_rm_free_area_srv(l4_cap_idx_t rm, l4_addr_t addr) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  return x->free_area(addr);
}

int
l4re_rm_attach_srv(l4_cap_idx_t rm, void **start, unsigned long size,
                   l4re_rm_flags_t flags, l4re_ds_t mem,
                   l4re_rm_offset_t offs,
                   unsigned char align) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  auto _mem = L4::Ipc::Cap<L4Re::Dataspace>::from_ci(mem);
  return x->attach(start, size, L4Re::Rm::Flags(flags), _mem, offs, align);
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
                 unsigned long *size,
                 l4re_rm_offset_t *offset,
                 l4re_rm_flags_t *flags, l4re_ds_t *m) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  L4::Cap<L4Re::Dataspace> mm(L4_INVALID_CAP);
  L4Re::Rm::Flags f;
  int r = x->find(addr, size, offset, &f, &mm);
  *flags = f.raw;
  *m = mm.cap();
  return r;
}

void
l4re_rm_show_lists_srv(l4_cap_idx_t rm) L4_NOTHROW
{
  L4::Cap<L4Re::Debug_obj> d(rm);
  d->debug(0); // XXX: use enum
}

int
l4re_rm_get_info_srv(l4_cap_idx_t rm, l4_addr_t addr,
                     char *name, unsigned int len,
                     l4re_rm_offset_t *backing_offset) L4_NOTHROW
{
  L4::Cap<L4Re::Rm> x(rm);
  L4::Ipc::String<char> str(len, name);
  return x->get_info(addr, str, *backing_offset);
}
