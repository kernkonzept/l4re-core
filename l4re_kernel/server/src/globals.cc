/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "region.h"
#include "globals.h"
#include <cstdlib>

// internal uclibc symbol for ENV
extern char const **__environ;

namespace Global
{
  L4::Cap<L4Re::Mem_alloc> allocator(L4::Cap_base::No_init);
  cxx::Static_container<Region_map> local_rm;
  cxx::Static_container<Cap_alloc> cap_alloc;
  char const *const *argv;
  char const *const *envp;
  int argc;
  l4re_aux_t *l4re_aux;
  bool Init_globals::_initialized;

  void Init_globals::init()
  {
    envp = __environ;
    l4_umword_t const *auxp = reinterpret_cast<l4_umword_t const *>(envp);
    while (*auxp)
      ++auxp;

    ++auxp;

    while (*auxp)
      {
        if (*auxp == 0xf0)
          l4re_aux = (l4re_aux_t*)auxp[1];
        auxp += 2;
      }

    L4Re::Env *env = const_cast<L4Re::Env*>(L4Re::Env::env());
    cap_alloc.construct(env->first_free_cap());
    env->first_free_cap(env->first_free_cap() + Global::Max_local_rm_caps);
    L4::Cap<L4Re::Mem_alloc> obj = env->mem_alloc();

    if (!obj.is_valid())
      {
        Err(Err::Fatal).printf("could not find the base memory allocator\n");
        exit(-1);
      }

    allocator = obj;
    local_rm.construct();
    local_rm->init();
  }
};
