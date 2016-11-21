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

/* Avoid warnings here for internals... */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

long
l4re_ma_free_srv(l4_cap_idx_t srv, l4re_ds_t const mem) L4_NOTHROW
{
  L4::Cap<L4Re::Mem_alloc> x(srv);
  L4::Cap<L4Re::Dataspace> ds(mem);
  return x->free(ds);
}
