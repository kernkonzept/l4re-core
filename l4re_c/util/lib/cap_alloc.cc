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
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/util/cap_alloc>

l4_cap_idx_t l4re_util_cap_alloc(void) L4_NOTHROW
{
  return L4Re::Util::cap_alloc.alloc<void>().cap();
}

void l4re_util_cap_free(l4_cap_idx_t cap) L4_NOTHROW
{
  L4Re::Util::cap_alloc.free(L4::Cap<void>(cap));
}

void l4re_util_cap_free_um(l4_cap_idx_t cap) L4_NOTHROW
{
  L4Re::Util::cap_alloc.free(L4::Cap<void>(cap), L4Re::This_task);
}

long
l4re_util_cap_last(void) L4_NOTHROW
{
  return L4Re::Util::cap_alloc.last();
}
