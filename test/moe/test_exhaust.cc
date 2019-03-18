/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test exhaustion of global resources (memory, capabilities).
 */

#include <climits>
#include <l4/re/env>
#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>

#include "moe_helpers.h"


struct TestExhaust : ::testing::Test {};

/**
 * Moe does not crash when all physical memory is exhaused.
 *
 * After freeing memory after an out-of-memory condition, allocations
 * should work again.
 */
TEST_F(TestExhaust, ExhaustMemory)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

    {
      auto hd = make_unique_del_cap<L4Re::Dataspace>();

      EXPECT_EQ(0, env->mem_alloc()->alloc(INT_MAX - 1, hd.get()));

      // now allocate until memory is exhausted
      for (l4_addr_t off = 0; off < INT_MAX - L4_PAGESIZE - 1;
           off += L4_PAGESIZE)
        {
          long ret = hd->allocate(off, L4_PAGESIZE);

          if (ret == -L4_ENOMEM)
            {
              EXPECT_GT(off, 0U);
              break;
            }
          ASSERT_EQ(L4_EOK, ret);
        }
    }

  // after freeing, we should be able to get more memory
  auto ds = make_unique_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, env->mem_alloc()->alloc(L4_PAGESIZE, ds.get(),
                                       L4Re::Mem_alloc::Continuous));
}

