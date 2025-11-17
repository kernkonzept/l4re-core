/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/re/cap_alloc>
#include <l4/re/mem_alloc>
#include <l4/re/env>
#include <l4/re/l4aux.h>
#include <l4/re/util/counting_cap_alloc>
#include <l4/cxx/static_container>
#include <l4/re/util/debug>

class Region_map_svr;

namespace Global
{
  enum
  {
    Max_local_rm_caps = 1024,
    Local_task_cap    = 0,
  };

  using Cap_alloc
    = L4Re::Cap_alloc_t<L4Re::Util::Counting_cap_alloc<
        L4Re::Util::Counter<unsigned char>,
        L4Re::Util::Dbg>>;

  extern L4::Cap<L4Re::Mem_alloc> allocator;
  extern cxx::Static_container<Region_map_svr> local_rm;
  extern cxx::Static_container<Cap_alloc> cap_alloc;
  extern char const *const *argv;
  extern char const *const *envp;
  extern int argc;
  extern l4re_aux_t *l4re_aux;
  extern bool has_pf_tramp;
};
