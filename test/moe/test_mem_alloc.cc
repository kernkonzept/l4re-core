/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test memory allocator of moe.
 */

#include <climits>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/dma_space>
#include <l4/re/error_helper>
#include <l4/re/debug>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"

struct TestMemAlloc : ::testing::Test {};

#ifndef NDEBUG
TEST_F(TestMemAlloc, Dump)
{
  auto dbg = L4::cap_reinterpret_cast<L4Re::Debug_obj>(env->mem_alloc());
  EXPECT_EQ(L4_EOK, dbg->debug(0));
}
#endif

TEST_F(TestMemAlloc, Simple)
{
  auto ds = make_auto_del_cap<L4Re::Dataspace>();

  ASSERT_EQ(0, env->mem_alloc()->alloc(1024, ds.get()));
  // we have a valid cap
  ASSERT_NE(0, env->task()->cap_valid(ds.get()).label());

  ASSERT_EQ(1024L, ds->size());

  char *start;
  // read-only
  ASSERT_EQ(0, env->rm()->attach(&start, 1024,
                                 L4Re::Rm::Search_addr | L4Re::Rm::Read_only,
                                 ds.get()));
  char rd = start[1023];
  // read/write
  ASSERT_EQ(0, env->rm()->attach(&start, 1024,
                                 L4Re::Rm::Search_addr,
                                 L4::Ipc::make_cap_rw(ds.get())));
  start[1023] = rd;

  // free dataspace by releasing cap
}

TEST_F(TestMemAlloc, OutOfRange)
{
  auto ds = make_auto_cap<L4Re::Dataspace>();

  EXPECT_EQ(-L4_ERANGE, env->mem_alloc()->alloc(0, ds.get()));
  EXPECT_EQ(-L4_ERANGE, env->mem_alloc()->alloc(~0UL, ds.get()));
}

TEST_F(TestMemAlloc, ExhaustQuotaMemory)
{
  auto cap = create_ma(10 * L4_PAGESIZE);

    {
      auto hd = make_auto_del_cap<L4Re::Dataspace>();

      EXPECT_EQ(0, cap->alloc(INT_MAX - 1, hd.get()));

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
  auto ds = make_auto_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, cap->alloc(L4_PAGESIZE, ds.get(),
                          L4Re::Mem_alloc::Continuous));
}


TEST_F(TestMemAlloc, Continuous)
{
  auto ds = make_auto_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, env->mem_alloc()->alloc(10 * L4_PAGESIZE, ds.get(),
                                       L4Re::Mem_alloc::Continuous));

  // continuous basically means that the memory is dma mappable,
  // so let's test that
  auto d = create_dma();
  L4Re::Dma_space::Dma_addr phys = 0;
  l4_size_t size = 10 * L4_PAGESIZE;
  ASSERT_EQ(0, d->map(L4::Ipc::make_cap_rw(ds.get()), 0, &size,
                      L4Re::Dma_space::Attributes::None,
                      L4Re::Dma_space::Direction::Bidirectional, &phys));
  ASSERT_EQ(0, d->unmap(phys, size,
                        L4Re::Dma_space::Attributes::None,
                        L4Re::Dma_space::Direction::Bidirectional));
}

TEST_F(TestMemAlloc, ContinuousHuge)
{
  // overcommit for continous memory is not possible
  auto ds = make_auto_cap<L4Re::Dataspace>();
  ASSERT_EQ(-L4_ENOMEM, env->mem_alloc()->alloc(1UL << 30, ds.get(),
                                                L4Re::Mem_alloc::Continuous));
}

TEST_F(TestMemAlloc, ContinuousMax)
{
  // this test assumes that the test environment is limited to 1GB of RAM
  auto ds = make_auto_cap<L4Re::Dataspace>();
  l4_ssize_t size = -0x4000000;

  ASSERT_EQ(0, env->mem_alloc()->alloc(size, ds.get(),
                                                L4Re::Mem_alloc::Continuous));

  // at least 'size' bytes must be left after the allocation
  l4_size_t mem = 1UL << 30;
  ASSERT_GT(mem, ds->size() - size - 1UL);
}

TEST_F(TestMemAlloc, NoncontMax)
{
  // this test assumes that the test environment is limited to 1GB of RAM
  auto ds = make_auto_cap<L4Re::Dataspace>();
  l4_ssize_t size = -0x4000000;

  ASSERT_EQ(-L4_ERANGE, env->mem_alloc()->alloc(size, ds.get()));
}

TEST_F(TestMemAlloc, Superpages)
{
  auto ds = make_auto_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, env->mem_alloc()->alloc(2 * L4_SUPERPAGESIZE, ds.get(),
                                       L4Re::Mem_alloc::Super_pages));

  char *start;
  ASSERT_EQ(0, env->rm()->attach(&start, L4_SUPERPAGESIZE,
                                 L4Re::Rm::Search_addr, ds.get()));
}
