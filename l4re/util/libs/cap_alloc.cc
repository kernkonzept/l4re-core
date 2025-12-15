/*!
 * \file
 * \brief  Capability allocator
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/crtn/initpriorities.h>
#include <l4/re/cap_alloc>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/debug>

#define INIT_PRIO __attribute__((init_priority(INIT_PRIO_L4RE_UTIL_CAP_ALLOC)))

namespace
{
  static constexpr unsigned Caps = CONFIG_L4RE_CAP_DFL_ALLOCATOR_MAX;

  L4Re::Util::_Cap_alloc::Storage<Caps> INIT_PRIO storage;
  L4Re::Util::Dbg INIT_PRIO dbg{0xffUL, "Cap_alloc", 0};

  long get_free_caps()
  {
    long ret = L4Re::Env::env()->first_free_cap();
    l4re_env()->first_free_cap += Caps;
    return ret;
  }

  unsigned get_free_reply_caps()
  {
    unsigned ret = L4Re::Env::env()->first_free_reply_cap();
    l4re_env()->first_free_reply_cap += decltype(L4Re::Util::reply_cap_alloc)::Capacity;
    return ret;
  }
}

namespace L4Re {
  namespace Util {
    _Cap_alloc INIT_PRIO cap_alloc{Caps, &storage, get_free_caps(), &dbg};

    Def_reply_cap_alloc INIT_PRIO reply_cap_alloc{get_free_reply_caps()};
  }
#ifndef SHARED
  Cap_alloc *virt_cap_alloc = &Util::cap_alloc;
#else
  // defined in ldso in the case of shared libs
  extern Cap_alloc *__rtld_l4re_virt_cap_alloc __attribute__((weak));

  // however, we have to set it to our cap allocator now
  // to enable the VFS to use the application cap allocator
  static void __attribute__((constructor))
  setup()
  {
    if (&__rtld_l4re_virt_cap_alloc)
      __rtld_l4re_virt_cap_alloc = &L4Re::Util::cap_alloc;
  }
#endif
}
