/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "region.h"
#include "globals.h"
#include <stdlib.h>
#include <l4/sys/compiler.h>
#include <l4/crtn/initpriorities.h>
#include <l4/re/util/bitmap_cap_alloc>

// internal uclibc symbol for ENV
extern char const **__environ;

namespace Global
{
  using Cap_alloc = L4Re::Util::Cap_alloc<Max_local_rm_caps>;

  L4::Cap<L4Re::Mem_alloc> allocator(L4::Cap_base::No_init);
  cxx::Static_container<Region_map> local_rm;
  // Accessed through the `cap_alloc` adapter below.
  static cxx::Static_container<Cap_alloc> bitmap_cap_alloc;
  L4Re::Cap_alloc *cap_alloc;
  char const *const *argv;
  char const *const *envp;
  int argc;
  l4re_aux_t *l4re_aux;

  void init();
  void init()
  {
    envp = __environ;
    l4_umword_t const *auxp = reinterpret_cast<l4_umword_t const *>(envp);
    while (*auxp)
      ++auxp;

    ++auxp;

    while (*auxp)
      {
        if (*auxp == 0xf0)
          l4re_aux = reinterpret_cast<l4re_aux_t *>(auxp[1]);
        auxp += 2;
      }

    L4Re::Env *env = const_cast<L4Re::Env*>(L4Re::Env::env());
    bitmap_cap_alloc.construct(env->first_free_cap());
    cap_alloc = L4Re::Cap_alloc::get_cap_alloc(*bitmap_cap_alloc);
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

  // Must be initialized as early as possible. Newer gcc versions require
  // malloc() to be available during the initial frame_dummy() call which calls
  // _register_frame_info(). Our malloc() implementation needs cap_alloc().
  //
  // See also INIT_PRIO_L4RE_UTIL_CAP_ALLOC.
  L4_DECLARE_CONSTRUCTOR(init, INIT_PRIO_EARLY)
};
