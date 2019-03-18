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

#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>

#include "moe_helpers.h"

struct TestMemAlloc : ::testing::Test
{
  /**
   * Return a large size that is still accepted by Moe for memory allocations.
   */
  l4_ssize_t huge_mem_size()
  {
    unsigned shiftwidth = sizeof(l4_addr_t) == 4
                          ? 31    // less than 2GB for 32bit
                          : 38;   // 256GB for 64bit

    return (l4_ssize_t) ((1UL << shiftwidth) - L4_PAGESIZE);
  }
};

#ifndef NDEBUG
/**
 * Memory allocation statistics can be dumped for debug purposes.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, Dump)
{
  TAP_COMP_FUNC("Moe", "L4Re::Debug_obj.debug");

  auto dbg = L4::cap_reinterpret_cast<L4Re::Debug_obj>(env->mem_alloc());
  EXPECT_EQ(L4_EOK, dbg->debug(0));
}
#endif

/**
 * A regular dataspace can be created, mapped for reading and remapped
 * for writing.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, Simple)
{
  TAP_COMP_FUNC ("Moe", "L4Re::Mem_alloc.alloc");
  TAP_COMP_FUNC2("Moe", "L4Re::Rm.attach");

  auto ds = make_unique_del_cap<L4Re::Dataspace>();

  ASSERT_EQ(0, env->mem_alloc()->alloc(1024, ds.get()));
  // we have a valid cap
  ASSERT_NE(0, env->task()->cap_valid(ds.get()).label());

  ASSERT_EQ(1024UL, ds->size());

  // read-only
  L4Re::Rm::Unique_region<char const *> ro_region;
  ASSERT_EQ(0, env->rm()->attach(&ro_region, 1024,
                                 L4Re::Rm::Search_addr | L4Re::Rm::Read_only,
                                 ds.get()));
  char rd = ro_region.get()[1023];
  // read/write
  L4Re::Rm::Unique_region<char *> rw_region;
  ASSERT_EQ(0, env->rm()->attach(&rw_region, 1024,
                                 L4Re::Rm::Search_addr,
                                 L4::Ipc::make_cap_rw(ds.get())));
  rw_region.get()[1023] = rd;

  // free dataspace by releasing cap
}

/**
 * A dataspace may not be created with zero or infinitely large size.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, OutOfRange)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

  auto ds = make_unique_cap<L4Re::Dataspace>();

  EXPECT_EQ(-L4_ERANGE, env->mem_alloc()->alloc(0, ds.get()));
  EXPECT_EQ(-L4_ERANGE, env->mem_alloc()->alloc(~0UL, ds.get()));
}

/**
 * When a dataspace is deleted its allocated memory is freed.
 *
 * The test allocates pages of a dataspace until moe reports that
 * the memory is used up, i.e. there is no more quota in the factory
 * the dataspace belongs to. Then it deletes the dataspace.
 * If the memory was freed correctly, it should be possible to create
 * and allocate a new dataspace afterwards.
 *
 * \see L4::Factory, L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, ExhaustQuotaMemory)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

  auto cap = create_ma(10 * L4_PAGESIZE);

    {
      auto hd = make_unique_del_cap<L4Re::Dataspace>();

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
  auto ds = make_unique_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, cap->alloc(L4_PAGESIZE, ds.get(),
                          L4Re::Mem_alloc::Continuous));
}

/**
 * A continuous dataspace can be allocated and mapped.
 *
 * \note Currently there is only one way to test if a dataspace is
 *       continuous and that is by mapping it into physical DMA space.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, Continuous)
{
  TAP_COMP_FUNC ("Moe", "L4Re::Mem_alloc.alloc");
  TAP_COMP_FUNC2("Moe", "L4Re::Dataspace.map");

  auto ds = make_unique_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, env->mem_alloc()->alloc(10 * L4_PAGESIZE, ds.get(),
                                       L4Re::Mem_alloc::Continuous));

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

/**
 * A continuous dataspace cannot be created with a larger size than the
 * available physical memory.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, ContinuousHuge)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

  // overcommit for continous memory is not possible
  auto ds = make_unique_cap<L4Re::Dataspace>();
  auto ds2 = make_unique_cap<L4Re::Dataspace>();
  auto ret = env->mem_alloc()->alloc(huge_mem_size(), ds.get(),
                                     L4Re::Mem_alloc::Continuous);
  if (ret == L4_EOK)
    ret = env->mem_alloc()->alloc(huge_mem_size(), ds2.get(),
                                  L4Re::Mem_alloc::Continuous);
  ASSERT_L4ERR(L4_ENOMEM, ret)
    << "Allocating more memory than physically available in a "
       "contiguous region fails.";
}

/**
 * When allocating a continuous dataspace with a negative size, then
 * enough memory will be left to allocate a dataspace of that size.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, ContinuousMax)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

  auto ds = make_unique_cap<L4Re::Dataspace>();
  l4_size_t left_size = 10 * L4_PAGESIZE;
  l4_ssize_t size = -((l4_ssize_t) left_size);

  ASSERT_L4OK(env->mem_alloc()->alloc(size, ds.get(),
                                      L4Re::Mem_alloc::Continuous));

  // at least 'left_size' bytes must be left after the allocation
  auto ds2 = make_unique_cap<L4Re::Dataspace>();
  L4Re::Rm::Unique_region<char *> ds2_region;
  ASSERT_L4OK(env->mem_alloc()->alloc(left_size, ds2.get()));
  ASSERT_L4OK(env->rm()->attach(&ds2_region, left_size,
                                L4Re::Rm::Search_addr | L4Re::Rm::Eager_map,
                                ds2.get()));
}

/**
 * Regular dataspaces cannot be created with a negative size.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, NoncontMax)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

  auto ds = make_unique_cap<L4Re::Dataspace>();
  l4_ssize_t size = -0x4000000;

  ASSERT_EQ(-L4_ERANGE, env->mem_alloc()->alloc(size, ds.get()));
}

/**
 * Dataspaces might be created to map super pages only.
 *
 * \todo confirm that only superpages are mapped.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_F(TestMemAlloc, Superpages)
{
  TAP_COMP_FUNC ("Moe", "L4Re::Mem_alloc.alloc");
  TAP_COMP_FUNC2("Moe", "L4Re::Rm.attach");

  auto ds = make_unique_del_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, env->mem_alloc()->alloc(2 * L4_SUPERPAGESIZE, ds.get(),
                                       L4Re::Mem_alloc::Super_pages));

  L4Re::Rm::Unique_region<char *> ds_region;
  ASSERT_EQ(0, env->rm()->attach(&ds_region, L4_SUPERPAGESIZE,
                                 L4Re::Rm::Search_addr, ds.get()));
}
