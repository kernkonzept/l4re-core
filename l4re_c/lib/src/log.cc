/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/env>
#include <l4/re/c/log.h>

L4_CV void
l4re_log_print_srv(const l4_cap_idx_t logcap,
                   char const *string) L4_NOTHROW
{
  L4::Cap<L4Re::Log>    x(logcap);
  x->print(string);
}

L4_CV void
l4re_log_printn_srv(const l4_cap_idx_t logcap,
                    char const *string, int len) L4_NOTHROW
{
  L4::Cap<L4Re::Log>    x(logcap);
  x->printn(string, len);
}
