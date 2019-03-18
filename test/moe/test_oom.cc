/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test out-of-memory behaviour of moe. (Emergency pool allocation.)
 *
 * GCC 4.9: moe does not crash and returns error.
 * GCC 5.3: moe terminates with 127
 * GCC 5.4: moe terminates with 127
 * GCC 6.3.1: moe terminates with 127
 */

#include <l4/re/dataspace>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>

#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>

/**
 * Acquire as much memory as possible to generate an out-of-memory state in
 * moe. If moe replied with ENOMEM, free all dataspace capabilities and try
 * again.
 * Expectation: Moe did not crash.
 */
TEST(TestOom, OutOfMemory)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

  constexpr int MAX = 100;
  {
    L4Re::Util::Unique_del_cap<L4Re::Dataspace> caparr[MAX];
    int i = 0;
    for (i = 0; i < MAX; ++i)
      {
        caparr[i] =
          L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
        long ret =
          L4Re::Env::env()->mem_alloc()->alloc(-1, caparr[i].get(),
                                               L4Re::Mem_alloc::Continuous);
        if (ret == -L4_ENOMEM)
          break;
      }

    ASSERT_LT(i, MAX) << "Out-of-memory situation not reached.";
  }

  auto cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
  ASSERT_L4OK(L4Re::Env::env()->mem_alloc()->alloc(-1, cap.get(),
                                                   L4Re::Mem_alloc::Continuous));
}
