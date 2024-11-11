/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
