/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/c/util/kumem_alloc.h>
#include <l4/re/util/kumem_alloc>

L4_CV int
l4re_util_kumem_alloc(l4_addr_t *mem, unsigned pages_order,
                      l4_cap_idx_t task, l4_cap_idx_t regmgr) L4_NOTHROW
{
  L4::Cap<L4::Task> t(task);
  L4::Cap<L4Re::Rm> r(regmgr);

  return L4Re::Util::kumem_alloc(mem, pages_order, t, r);
}
