
/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test dma_space implementation of moe.
 * Only preliminary tests because Moe has only a stub implementation.
 */

#include <l4/re/env>
#include <l4/re/dma_space>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/error_helper>

#include <l4/atkins/tap/main>
#include <l4/atkins/debug>

#include "moe_helpers.h"

struct TestDmaSpace : testing::Test {};

TEST_F(TestDmaSpace, MapUnmapSinglePage)
{
  auto dma = create_dma();
  auto ds = create_cont_ds(L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  ASSERT_EQ(0, dma->map(L4::Ipc::make_cap_rw(ds.get()), 0, &sz,
                        L4Re::Dma_space::Attributes::None,
                        L4Re::Dma_space::Bidirectional, &addr));
  EXPECT_GE(L4_PAGESIZE, sz);
  EXPECT_EQ(0U, addr % L4_PAGESIZE);

  ASSERT_EQ(0, dma->unmap(addr, sz,
                          L4Re::Dma_space::Attributes::None,
                          L4Re::Dma_space::Bidirectional));
}


TEST_F(TestDmaSpace, MapUnmapWithOffset)
{
  auto dma = create_dma();
  auto ds = create_cont_ds(4 * L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  ASSERT_EQ(0, dma->map(L4::Ipc::make_cap_rw(ds.get()), 2 * L4_PAGESIZE, &sz,
                        L4Re::Dma_space::Attributes::None,
                        L4Re::Dma_space::Bidirectional, &addr));
  EXPECT_GE(2 * L4_PAGESIZE, sz);
  EXPECT_EQ(0U, addr % L4_PAGESIZE);

  ASSERT_EQ(0, dma->unmap(addr, sz,
                          L4Re::Dma_space::Attributes::None,
                          L4Re::Dma_space::Bidirectional));
}

TEST_F(TestDmaSpace, MapUnmapUnevenOffset)
{
  auto dma = create_dma();
  auto ds = create_cont_ds(4 * L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  ASSERT_EQ(0, dma->map(L4::Ipc::make_cap_rw(ds.get()), 100, &sz,
                        L4Re::Dma_space::Attributes::None,
                        L4Re::Dma_space::Bidirectional, &addr));
  EXPECT_GE(2 * L4_PAGESIZE, sz);
  EXPECT_EQ(0U, (addr - 100) % L4_PAGESIZE);

  ASSERT_EQ(0, dma->unmap(addr, sz,
                          L4Re::Dma_space::Attributes::None,
                          L4Re::Dma_space::Bidirectional));
}

TEST_F(TestDmaSpace, ReadOnlyDataspace)
{
  auto dma = create_dma();
  auto ds = create_cont_ds(L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  EXPECT_EQ(-L4_EPERM, dma->map(ds.get(), 0, &sz,
                                L4Re::Dma_space::Attributes::None,
                                L4Re::Dma_space::None, &addr));
}

TEST_F(TestDmaSpace, RegularDataspace)
{
  auto dma = create_dma();
  auto ds = create_ds(L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  EXPECT_EQ(-L4_EINVAL, dma->map(L4::Ipc::make_cap_rw(ds.get()), 0, &sz,
                                L4Re::Dma_space::Attributes::None,
                                L4Re::Dma_space::None, &addr));
}

TEST_F(TestDmaSpace, InvalidDataspace)
{
  auto dma = create_dma();

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  auto badcap = L4::cap_reinterpret_cast<L4Re::Dataspace>(env->factory());
  EXPECT_EQ(-L4_EINVAL, dma->map(L4::Ipc::make_cap_rw(badcap),
                                 0, &sz, L4Re::Dma_space::Attributes::None,
                                 L4Re::Dma_space::None, &addr));
}

TEST_F(TestDmaSpace, ExhaustQuotaMoeStructures)
{
  auto cap = create_fab(2 * L4_PAGESIZE);

  // Create dataspaces without deleting them until we are out of memory
  std::vector<L4Re::Util::Ref_cap<L4Re::Dma_space>::Cap> dslist;

  for (;;)
    {
      auto ds = make_ref_cap<L4Re::Dma_space>();

      long ret = l4_error(cap->create(ds.get()));
      if (ret == L4_EOK)
        dslist.push_back(ds);
      else
        {
          ASSERT_EQ(-L4_ENOMEM, ret);
          ASSERT_FALSE(dslist.empty());
          // free the previously allocated dataspace for more memory
          dslist.pop_back();
          break;
        }
    }

  // after freeing, we should be able to get more memory
  auto ds = make_unique_cap<L4Re::Dma_space>();
  ASSERT_EQ(0, l4_error(cap->create(ds.get())));
}
