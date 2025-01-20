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
#include <l4/cxx/static_container>

class Region_map;

namespace Global
{
  enum
  {
    Max_local_rm_caps = 1024,
    Local_task_cap    = 0,
  };
  extern L4::Cap<L4Re::Mem_alloc> allocator;
  extern cxx::Static_container<Region_map> local_rm;
  extern L4Re::Cap_alloc *cap_alloc;
  extern char const *const *argv;
  extern char const *const *envp;
  extern int argc;
  extern l4re_aux_t *l4re_aux;
};
