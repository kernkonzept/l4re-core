/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/c/namespace.h>

long l4re_ns_query_to_srv(l4_cap_idx_t srv, char const *name,
                          l4_cap_idx_t const cap, int timeout) L4_NOTHROW
{
  L4::Cap<L4Re::Namespace> x(srv);
  return x->query(name, L4::Cap<void>(cap), timeout);
}

long l4re_ns_register_obj_srv(l4_cap_idx_t srv,
                              const char* name, l4_cap_idx_t const obj,
                              unsigned flags) L4_NOTHROW
{
  L4::Cap<L4Re::Namespace> x(srv);
  return x->register_obj(name, L4::Ipc::Cap<void>::from_ci(obj), flags);
}
