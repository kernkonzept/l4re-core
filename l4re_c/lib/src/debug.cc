/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/env>
#include <l4/re/debug>
#include <l4/re/c/debug.h>

l4_ret_t
l4re_debug_obj_debug(l4_cap_idx_t srv, unsigned long function) L4_NOTHROW
{
  L4::Cap<L4Re::Debug_obj> x(srv);
  return x->debug(function);
}
