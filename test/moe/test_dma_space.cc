
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
#include <l4/atkins/l4_assert>

#include "moe_helpers.h"

struct TestDmaSpace
: testing::Test,
  testing::WithParamInterface<L4Re::Dma_space::Space_attribs>
{
  virtual void SetUp()
  {
    if (!(GetParam() & L4Re::Dma_space::Space_attrib::Phys_space))
      {
        dma_task = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4::Task>(),
                                "Create capability for kernel DMA space.");
        auto f = env->factory();
        if (l4_error(f->create(dma_task.get(), L4_PROTO_DMA_SPACE)) < 0)
          RecordProperty("SKIP", "No IOMMU available.");

      }

    dma = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dma_space>(),
                       "Create capability for moe DMA space.");
    L4Re::chksys(env->user_factory()->create(dma.get()),
                 "Create moe DMA space.");
    L4Re::chksys(dma->associate(L4::Ipc::make_cap_rws(dma_task.get()),
                                GetParam()),
                 "Associate moe DMA space.");
  }

  L4Re::Util::Unique_del_cap<L4::Task> dma_task;
  L4Re::Util::Unique_del_cap<L4Re::Dma_space> dma;
};

L4Re::Dma_space::Space_attribs const Dma_types[] =
{
  L4Re::Dma_space::Space_attrib::Phys_space,  /// DMA space of physical RAM.
  L4Re::Dma_space::Space_attribs::None        /// Regular DMA space.
};

static INSTANTIATE_TEST_CASE_P(DmaSpace, TestDmaSpace,
                               testing::ValuesIn(Dma_types));

TEST_P(TestDmaSpace, MapUnmapSinglePage)
{
  auto ds = create_cont_ds(L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  ASSERT_L4OK(dma->map(L4::Ipc::make_cap_rw(ds.get()), 0, &sz,
                       L4Re::Dma_space::Attributes::None,
                       L4Re::Dma_space::Bidirectional, &addr));
  EXPECT_GE(L4_PAGESIZE, sz);
  EXPECT_EQ(0U, addr % L4_PAGESIZE);

  ASSERT_L4OK(dma->unmap(addr, sz,
                         L4Re::Dma_space::Attributes::None,
                         L4Re::Dma_space::Bidirectional));
}


TEST_P(TestDmaSpace, MapUnmapWithOffset)
{
  auto ds = create_cont_ds(4 * L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  ASSERT_L4OK(dma->map(L4::Ipc::make_cap_rw(ds.get()), 2 * L4_PAGESIZE, &sz,
                       L4Re::Dma_space::Attributes::None,
                       L4Re::Dma_space::Bidirectional, &addr));
  EXPECT_GE(2 * L4_PAGESIZE, sz);
  EXPECT_EQ(0U, addr % L4_PAGESIZE);

  ASSERT_L4OK(dma->unmap(addr, sz,
                         L4Re::Dma_space::Attributes::None,
                         L4Re::Dma_space::Bidirectional));
}

TEST_P(TestDmaSpace, MapUnmapUnevenOffset)
{
  auto ds = create_cont_ds(4 * L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  ASSERT_L4OK(dma->map(L4::Ipc::make_cap_rw(ds.get()), 100, &sz,
                       L4Re::Dma_space::Attributes::None,
                       L4Re::Dma_space::Bidirectional, &addr));
  EXPECT_GE(2 * L4_PAGESIZE, sz);
  EXPECT_EQ(0U, (addr - 100) % L4_PAGESIZE);

  ASSERT_L4OK(dma->unmap(addr, sz,
                         L4Re::Dma_space::Attributes::None,
                         L4Re::Dma_space::Bidirectional));
}

TEST_P(TestDmaSpace, ReadOnlyDataspace)
{
  auto ds = create_cont_ds(L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  EXPECT_L4ERR(L4_EPERM, dma->map(ds.get(), 0, &sz,
                                  L4Re::Dma_space::Attributes::None,
                                  L4Re::Dma_space::None, &addr));
}

TEST_P(TestDmaSpace, RegularDataspace)
{
  auto ds = create_ds(L4_PAGESIZE);

  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  long ret = dma->map(L4::Ipc::make_cap_rw(ds.get()), 0, &sz,
                      L4Re::Dma_space::Attributes::None,
                      L4Re::Dma_space::None, &addr);

  if (GetParam() & L4Re::Dma_space::Space_attrib::Phys_space)
    {
      EXPECT_L4ERR(L4_EINVAL, ret)
        << "Physical DMA spaces cannot map a regular dataspace.";
    }
  else
    {
      EXPECT_L4OK(ret)
        << "Regular DMA spaces are able to map a regular dataspace.";
    }
}

TEST_P(TestDmaSpace, InvalidDataspace)
{
  l4_size_t sz = L4_PAGESIZE;
  L4Re::Dma_space::Dma_addr addr = 0;
  auto badcap = L4::cap_reinterpret_cast<L4Re::Dataspace>(env->factory());
  EXPECT_L4ERR(L4_EINVAL, dma->map(L4::Ipc::make_cap_rw(badcap),
                                   0, &sz, L4Re::Dma_space::Attributes::None,
                                   L4Re::Dma_space::None, &addr));
}

TEST_P(TestDmaSpace, ExhaustQuotaMoeStructures)
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
          ASSERT_L4ERR(L4_ENOMEM, ret);
          ASSERT_FALSE(dslist.empty());
          // free the previously allocated dataspace for more memory
          dslist.pop_back();
          break;
        }
    }

  // after freeing, we should be able to get more memory
  auto ds = make_unique_cap<L4Re::Dma_space>();
  ASSERT_L4OK(cap->create(ds.get()));
}
