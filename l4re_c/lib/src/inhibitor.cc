/*
 * (c) 2014 Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is licensed under the terms of the GNU Lesser General
 * Public Licence 2.1.
 * See the file COPYING-LGPL-2.1 for details.
 */

#include <l4/sys/capability>
#include <l4/re/c/inhibitor.h>
#include <l4/re/inhibitor>


L4_CV long L4_EXPORT
l4re_inhibitor_acquire(l4_cap_idx_t cap, l4_umword_t id,
                       char const *reason)
{
  return L4::Cap<L4Re::Inhibitor>(cap)->acquire(id, reason);
}

L4_CV long L4_EXPORT
l4re_inhibitor_release(l4_cap_idx_t cap, l4_umword_t id)
{
  return L4::Cap<L4Re::Inhibitor>(cap)->release(id);
}

L4_CV long L4_EXPORT
l4re_inhibitor_next_lock_info(l4_cap_idx_t cap, char *name,
                              unsigned len, l4_mword_t current_id)
{
  return  L4::Cap<L4Re::Inhibitor>(cap)->next_lock_info(name, len, current_id);
}

