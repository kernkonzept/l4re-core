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

#include <l4/bid_config.h>
#include <l4/crtn/initpriorities.h>
#include <l4/re/cap_alloc>
#include <l4/re/dataspace>
#include <l4/re/env>
#include <l4/re/mem_alloc>
#include <l4/re/util/cap_alloc>
#include <l4/sys/assert.h>

namespace
{
  struct Ca : L4Re::Cap_alloc_t<L4Re::Util::_Cap_alloc>
  {
    typedef L4Re::Cap_alloc_t<L4Re::Util::_Cap_alloc> Base;

#ifdef CONFIG_MMU
    enum { Caps = 4096 };
#else
    enum { Caps = 64 };
#endif

    typedef L4Re::Util::_Cap_alloc::Storage<Caps> Storage;

    static void *_alloc_storage()
    {
#if !defined(CONFIG_MMU)
      static Storage __cap_storage;
      return &__cap_storage;
#else
      L4::Cap<L4Re::Dataspace> ds(L4::Cap<L4Re::Dataspace>::No_init);
      L4Re::Env const *e = L4Re::Env::env();
      ds = L4::Cap<L4Re::Dataspace>(e->first_free_cap() << L4_CAP_SHIFT);
      l4_check(e->mem_alloc()->alloc(sizeof(Storage), ds) >= 0);
      void *a = 0;
      l4_check(e->rm()->attach(&a, sizeof(Storage),
                               L4Re::Rm::F::Search_addr | L4Re::Rm::F::RW,
                               L4::Ipc::make_cap_rw(_ds)) >= 0);
    }
#endif
    }

    Ca()
    : Base(Caps, _alloc_storage(), L4Re::Env::env()->first_free_cap() + 1)
    {}
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
