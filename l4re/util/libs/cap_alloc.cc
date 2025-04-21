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

namespace
{
  struct Ca
  {
    enum { Caps = CONFIG_L4RE_CAP_DFL_ALLOCATOR_MAX };

    L4Re::Util::_Cap_alloc::Storage<Caps> storage;
    L4Re::Util::Dbg _dbg;
    L4Re::Util::_Cap_alloc alloc;

    Ca()
    : _dbg{0xffUL, "Cap_alloc", 0},
      alloc{Caps, &storage,
            static_cast<long>(L4Re::Env::env()->first_free_cap()), &_dbg}
    { l4re_env()->first_free_cap += Caps; }
  };

  Ca __attribute__((init_priority(INIT_PRIO_L4RE_UTIL_CAP_ALLOC))) __cap_alloc;
}

namespace L4Re {
  namespace Util {
    _Cap_alloc &cap_alloc = __cap_alloc.alloc;
  }
#ifndef SHARED
  Cap_alloc *virt_cap_alloc = &__cap_alloc.alloc;
#else
  // defined in ldso in the case of shared libs
  extern Cap_alloc *__rtld_l4re_virt_cap_alloc __attribute__((weak));

  // however, we have to set it to our cap allocator now
  // to enable the VFS to use the application cap allocator
  static void __attribute__((constructor))
  setup()
  {
    if (&__rtld_l4re_virt_cap_alloc)
      __rtld_l4re_virt_cap_alloc = &__cap_alloc.alloc;
  }
#endif
}
