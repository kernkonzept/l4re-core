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

#include <l4/sys/assert.h>
#include <l4/crtn/initpriorities.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>

//#define L4RE_STATIC_CAP_ALLOC
#if defined(L4RE_STATIC_CAP_ALLOC)

namespace
{
  L4Re::Util::Cap_alloc<4096> __attribute__((init_priority(INIT_PRIO_L4RE_UTIL_CAP_ALLOC))) __cap_alloc(L4Re::Env::env()->first_free_cap());
};
#else

#include <l4/re/dataspace>
#include <l4/re/mem_alloc>

namespace
{
  struct Ca : public L4Re::Util::_Cap_alloc
  {
    enum { Caps = 4096 };
    typedef L4Re::Util::_Cap_alloc::Counter_storage<Caps> Storage;

    L4::Cap<L4Re::Dataspace> _ds;
    Ca() : _ds(L4::Cap<L4Re::Dataspace>::No_init)
    {
      L4Re::Env const *e = L4Re::Env::env();
      _ds = L4::Cap<L4Re::Dataspace>(e->first_free_cap() << L4_CAP_SHIFT);
      l4_check(e->mem_alloc()->alloc(sizeof(Storage), _ds) >= 0);
      void *a = 0;
      l4_check(e->rm()->attach(&a, sizeof(Storage), L4Re::Rm::Search_addr,
                               L4::Ipc::make_cap_rw(_ds)) >= 0);
      setup(a, Caps, e->first_free_cap() + 1);
    }
  };

  Ca __attribute__((init_priority(INIT_PRIO_L4RE_UTIL_CAP_ALLOC))) __cap_alloc;
};
#endif


namespace L4Re { namespace Util {

_Cap_alloc &cap_alloc = __cap_alloc;

}}
