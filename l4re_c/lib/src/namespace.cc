/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/c/namespace.h>

l4_ret_t l4re_ns_query_to_srv(l4_cap_idx_t srv, char const *name,
                              l4_cap_idx_t const cap, int timeout) L4_NOTHROW
{
  L4::Cap<L4Re::Namespace> x(srv);
  return x->query(name, L4::Cap<void>(cap), timeout);
}

l4_ret_t l4re_ns_register_obj_srv(l4_cap_idx_t srv,
                                  const char* name, l4_cap_idx_t const obj,
                                  unsigned flags) L4_NOTHROW
{
  L4::Cap<L4Re::Namespace> x(srv);
  return x->register_obj(name, L4::Ipc::Cap<void>::from_ci(obj), flags);
}
