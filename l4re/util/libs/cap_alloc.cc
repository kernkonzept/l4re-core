/*!
 * \file   l4re/util/libs/cap_alloc.cc
 * \brief  Capability allactor
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/crtn/initpriorities.h>
#include <l4/re/cap_alloc>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/debug>

namespace
{
  struct Ca : L4Re::Cap_alloc_t<L4Re::Util::_Cap_alloc>
  {
    enum { Caps = 4096 };
    typedef L4Re::Util::_Cap_alloc::Counter_storage<Caps> Storage;

    L4Re::Util::Dbg _dbg;

    Ca() : _dbg(0xffUL, "Cap_alloc", 0)
    {
      static Storage __cap_storage;
      setup(&__cap_storage, Caps, L4Re::Env::env()->first_free_cap(), &_dbg);
      l4re_env()->first_free_cap += Caps;
    }
  };

  Ca __attribute__((init_priority(INIT_PRIO_L4RE_UTIL_CAP_ALLOC))) __cap_alloc;
}

namespace L4Re {
  namespace Util {
    _Cap_alloc &cap_alloc = __cap_alloc;
  }
#ifndef SHARED
  Cap_alloc *virt_cap_alloc = &__cap_alloc;
#else
  // defined in ldso in the case of shared libs
  extern Cap_alloc *__rtld_l4re_virt_cap_alloc __attribute__((weak));

  // however, we have to set it to our cap allocator now
  // to enable the VFS to use the application cap allocator
  static void __attribute__((constructor))
  setup()
  {
    if (&__rtld_l4re_virt_cap_alloc)
      __rtld_l4re_virt_cap_alloc = &__cap_alloc;
  }
#endif
}
