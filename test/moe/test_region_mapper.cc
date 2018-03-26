/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Tests for the region mapper type.
 */

#include <l4/re/util/cap_alloc>
#include <l4/re/util/shared_cap>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/sys/pager>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"

static Atkins::Dbg dbg{2};

/**
 * Fixture for region mapper tests.
 */
struct TestRm : testing::Test
{
  /**
   * Send a page fault to the region mapper and check that it was answered
   * positively.
   *
   * \param rm    Region mapper to use.
   * \param pfa   Faulting address.
   * \param start Send base of returned flex page.
   * \param size  Size of the returned flex page.
   */
  void good_pf(L4::Cap<L4Re::Rm> rm, l4_addr_t pfa,
               l4_addr_t start, unsigned long size)
  {
    L4::Ipc::Snd_fpage sndfpage;
    l4_mword_t result;
    dbg.printf("faking good page fault (0x%lx)\n", pfa);
    l4_msgtag_t msg = rm->page_fault(pfa, 0x1000, result,
                                     L4::Ipc::Rcv_fpage::mem(0, L4_WHOLE_ADDRESS_SPACE, 0),
                                     sndfpage);

    dbg.printf("good page fault (0x%lx) : 0x%lx/0x%lx\n",
               pfa, sndfpage.base_x(), sndfpage.snd_base());
    EXPECT_FALSE(msg.has_error());
    EXPECT_EQ(1U, msg.items());
    EXPECT_EQ(0U, msg.words());
    EXPECT_LE(start, sndfpage.snd_base());
    EXPECT_GE(start + size, sndfpage.snd_base() + (1 << sndfpage.rcv_order()));
  }

  /**
   * Send a page fault to the region mapper and check that it was denied.
   *
   * \param rm    Region mapper to use.
   * \param pfa   Faulting address.
   */
  l4_mword_t bad_pf(L4::Cap<L4Re::Rm> rm, l4_addr_t pfa)
  {
    L4::Ipc::Snd_fpage sndfpage;
    l4_mword_t result = 0;
    l4_msgtag_t msg = rm->page_fault(pfa, 0x1000, result, l4_fpage_all(),
                                     sndfpage);

    EXPECT_FALSE(msg.has_error());
    EXPECT_EQ(0U, msg.items());
    EXPECT_EQ(1U, msg.words());

    return result;
  }

  /**
   * Create a region mapper where all areas that are used by the test
   * are reserved.
   *
   * When testing page faults, we need to ensure that pages are mapped into
   * areas that are free. The region mapper that is returned by this
   * function has exactly the same free areas as the test's own
   * region mapper.
   */
  L4Re::Util::Unique_del_cap<L4Re::Rm>
  create_blocked_rm()
  {
    auto rm = create_rm();

    enum
    {
      /// get_regions() transfers the region list via UTCB message registers
      Region_list_size = sizeof(l4_msg_regs_t) / sizeof(L4Re::Rm::Region)
    };
    L4Re::Rm::Region region_list[Region_list_size];
    L4Re::Rm::Region const *rl;
    l4_addr_t addr = 0;
    long n;

    while ((n = env->rm()->get_regions(addr, &rl)) > 0)
    {
      // copy data returned in the UTCB
      assert(n <= Region_list_size);
      memcpy(region_list, rl, n * sizeof(L4Re::Rm::Region));

      for (int i = 0; i < n; ++i)
      {
        auto const *r = &region_list[i];
        l4_addr_t start = r->start;
        long ret = rm->reserve_area(&start, r->end - r->start);
        printf("Reserve: 0x%lx/0x%lx -> %lu\n", r->start, r->end, ret);
        L4Re::chksys(ret, "Reserve area in the new RM.");
        addr = r->end + 1;
      }
    }

    l4_fpage_t utcbs = env->utcb_area();

    l4_addr_t start = l4_fpage_memaddr(utcbs);
    rm->reserve_area(&start, l4_fpage_size(utcbs));

    return rm;
  }

};

/**
 * A region may be reserved at an address chosen by the server.
 * The address will not be at 0.
 *
 * \see L4Re::Rm.reserve_area
 */
TEST_F(TestRm, ReservePageSearch)
{
  auto rm = create_rm();

  unsigned long start = 0;

  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE, L4Re::Rm::Search_addr));
  EXPECT_NE(start, 0UL);

  // large reserve
  EXPECT_EQ(0, rm->reserve_area(&start, 1024 * L4_PAGESIZE,
                                L4Re::Rm::Search_addr));
}

/**
 * A region may be reserved at an address chosen by the caller.
 *
 * \see L4Re::Rm.reserve_area
 */
TEST_F(TestRm, ReservePageFixed)
{
  auto rm = create_rm();

  unsigned long start = 0x1000000;
  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE));
  EXPECT_EQ(start, 0x1000000UL);
}

/**
 * When a region address is chosen by the caller, it may not overlap
 * with existing regions.
 *
 * \see L4Re::Rm.reserve_area
 */
TEST_F(TestRm, ReserveTwice)
{
  auto rm = create_rm();

  unsigned long start = 0;

  EXPECT_EQ(0, rm->reserve_area(&start, 10 * L4_PAGESIZE, L4Re::Rm::Search_addr));
  EXPECT_NE(start, 0UL);

  unsigned long s = start;
  EXPECT_EQ(-L4_EADDRNOTAVAIL, rm->reserve_area(&s, 10 * L4_PAGESIZE));
  s = start + L4_PAGESIZE;
  EXPECT_EQ(-L4_EADDRNOTAVAIL, rm->reserve_area(&s, L4_PAGESIZE));
  s = start - L4_PAGESIZE;
  EXPECT_EQ(-L4_EADDRNOTAVAIL, rm->reserve_area(&s, 2 * L4_PAGESIZE));
  EXPECT_EQ(-L4_EADDRNOTAVAIL, rm->reserve_area(&s, 11 * L4_PAGESIZE));
  s = start + 9 * L4_PAGESIZE;
  EXPECT_EQ(-L4_EADDRNOTAVAIL, rm->reserve_area(&s, 2 * L4_PAGESIZE));
}

/**
 * The start address of a region does not need to be aligned to a
 * page boundary.
 *
 * \see L4Re::Rm.reserve_area
 */
TEST_F(TestRm, ReserveUnaligned)
{
  auto rm = create_rm();

  unsigned long start = 0x1000010;
  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE));
  EXPECT_EQ(start, 0x1000010UL);

  start = 0x1000000;
  EXPECT_EQ(0, rm->reserve_area(&start, 10));
}

/**
 * A region may have a size that is less than a page.
 *
 * \see L4Re::Rm.reserve_area
 */
TEST_F(TestRm, ReserveTiny)
{
  auto rm = create_rm();

  unsigned long start = 0;
  EXPECT_EQ(0, rm->reserve_area(&start, 10, L4Re::Rm::Search_addr));
  EXPECT_EQ(0UL, start % L4_PAGESIZE);
  EXPECT_EQ(0, rm->reserve_area(&start, 10, L4Re::Rm::Search_addr, 1));
}

/**
 * When a reserved area is deleted its allocated memory is freed.
 *
 * The test reserves areas until moe reports to be out of memory
 * and then deletes exactly one area. If the memory was freed correctly
 * it should be possible to reserve another area.
 *
 * \see L4Re:Rm.reserve_area, L4Re:Rm.free_area
 */
TEST_F(TestRm, ExhaustQuotaReserve)
{
  auto fab = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4::Factory>());
  L4Re::chksys(env->user_factory()->create_factory(fab.get(), 3 * L4_PAGESIZE));

  auto rm = create_rm(fab.get());

  unsigned long prev_start = 0;
  for (;;)
    {
      unsigned long start = 0;
      long ret = rm->reserve_area(&start, 10, L4Re::Rm::Search_addr, 1);
      if (ret == -L4_EADDRNOTAVAIL)
        break;

      ASSERT_EQ(ret, L4_EOK);
      ASSERT_GT(start, 0UL);

      prev_start = start;
    }

  ASSERT_GT(prev_start, 0UL);
  ASSERT_EQ(0, rm->free_area(prev_start));
  prev_start = 0;
  ASSERT_EQ(0, rm->reserve_area(&prev_start, 10, L4Re::Rm::Search_addr, 1));
}

/**
 * A previously reserved area may be freed.
 *
 * \see L4Re:Rm.reserve_area, L4Re:Rm.free_area
 */
TEST_F(TestRm, FreeAreaSimple)
{
  auto rm = create_rm();

  unsigned long start = 0;
  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE, L4Re::Rm::Search_addr));
  EXPECT_NE(start, 0UL);

  EXPECT_EQ(0, rm->free_area(start));
}

/**
 * Freeing an area that was not reserved fails.
 *
 * \see L4Re:Rm.free_area
 */
TEST_F(TestRm, FreeAreaNonExisting)
{
  auto rm = create_rm();

  EXPECT_EQ(-L4_ENOENT, rm->free_area(100 * L4_PAGESIZE));
}

/**
 * Region lookup is not permitted by moe.
 *
 * \see L4Re:Rm.find
 */
TEST_F(TestRm, FindNotAllowed)
{
  auto rm = create_rm();

  l4_addr_t addr = 0;
  unsigned long size = L4_PAGESIZE;
  l4_addr_t offset = 0;
  unsigned flags = 0;
  L4::Cap<L4Re::Dataspace> cap;
  EXPECT_EQ(-L4_EPERM, rm->find(&addr, &size, &offset, &flags, &cap));
}

/**
 * A complete dataspace may be attached to and detached from a region.
 *
 * \see L4Re:Rm.attach, L4Re::Rm.detach
 */
TEST_F(TestRm, AttachDetachFull)
{
  unsigned long sz = L4_PAGESIZE;
  auto rm = create_rm();
  auto ds = create_ds(0, sz);

  l4_addr_t start = 0;
  ASSERT_EQ(0, env->rm()->reserve_area(&start, sz, L4Re::Rm::Search_addr));

  ASSERT_EQ(0, rm->attach(&start, sz, 0, L4::Ipc::make_cap_rw(ds.get())));

  good_pf(rm.get(), start, start, sz);
  good_pf(rm.get(), start + sz - 8, start, sz);

  EXPECT_EQ(-1, bad_pf(rm.get(), start - 8));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz));

  L4::Cap<L4Re::Dataspace> oldds;
  ASSERT_EQ(0, rm->detach(start, &oldds));
  EXPECT_EQ(oldds.cap(), ds.cap());

  EXPECT_EQ(-1, bad_pf(rm.get(), start));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz - 1));

  env->rm()->free_area(sz);
}

/**
 * A part of a dataspace may be attached to and detached from a region.
 *
 * \see L4Re:Rm.attach, L4Re::Rm.detach
 */
TEST_F(TestRm, AttachDetachPartial)
{
  unsigned long sz = L4_PAGESIZE;
  auto rm = create_rm();
  auto ds = create_ds(0, sz * 4);

  l4_addr_t start = 0;
  ASSERT_EQ(0, env->rm()->reserve_area(&start, sz, L4Re::Rm::Search_addr));

  ASSERT_EQ(0, rm->attach(&start, sz, 0,
                          L4::Ipc::make_cap_rw(ds.get()), L4_PAGESIZE));

  good_pf(rm.get(), start, start, sz);
  good_pf(rm.get(), start + sz - 8, start, sz);

  EXPECT_EQ(-1, bad_pf(rm.get(), start - 8));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz));

  L4::Cap<L4Re::Dataspace> oldds;
  ASSERT_EQ(0, rm->detach(start, &oldds));
  EXPECT_EQ(oldds.cap(), ds.cap());

  EXPECT_EQ(-1, bad_pf(rm.get(), start));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz - 1));

  env->rm()->free_area(sz);
}

/**
 * A dataspace may be attached to a region that is larger than its size.
 *
 * \see L4Re:Rm.attach
 */
TEST_F(TestRm, AttachTooSmall)
{
  unsigned long sz = L4_PAGESIZE;
  auto rm = create_rm();
  auto ds = create_ds(sz);

  l4_addr_t start = 0;
  ASSERT_EQ(0, env->rm()->reserve_area(&start, 2 * sz, L4Re::Rm::Search_addr));

  ASSERT_EQ(0, rm->attach(&start, 2 * sz, 0, L4::Ipc::make_cap_rw(ds.get())));

  good_pf(rm.get(), start, start, 2 * sz);
  EXPECT_EQ(-1, bad_pf(rm.get(), start + 2 * sz - 8));

  EXPECT_EQ(-1, bad_pf(rm.get(), start - 8));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz));

  L4::Cap<L4Re::Dataspace> oldds;
  ASSERT_EQ(0, rm->detach(start, &oldds));
  EXPECT_EQ(oldds.cap(), ds.cap());

  EXPECT_EQ(-1, bad_pf(rm.get(), start));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz - 1));

  env->rm()->free_area(sz);
}

/**
 * When a dataspace is deleted after being attached to a region,
 * subsequent page faults on the region will result in an error.
 *
 * \see L4Re:Rm.attach
 */
TEST_F(TestRm, AttachRemoveDataspace)
{
  l4_addr_t start = 0;
  unsigned long sz;
  auto rm = create_blocked_rm();

    {
      auto ds = create_ds();
      sz = ds->size();

      ASSERT_EQ(0, rm->attach(&start, sz, L4Re::Rm::Search_addr,
                              L4::Ipc::make_cap_rw(ds.get())));
    }

  EXPECT_EQ(-1, bad_pf(rm.get(), start));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz - 1));

  L4::Cap<L4Re::Dataspace> oldds;
  ASSERT_EQ(0, rm->detach(start, &oldds));
}

/**
 * A dataspace remains attached to a region and accessible even when
 * all other capabilities to the dataspace are removed.
 *
 * \see L4Re:Rm.attach
 */
TEST_F(TestRm, AttachLooseDataspace)
{
  l4_addr_t start = 0;
  unsigned long sz;
  auto rm = create_blocked_rm();

    {
      // Cap should be removed without explicitly deleting.
      auto ds = L4Re::chkcap(L4Re::Util::make_unique_cap<L4Re::Dataspace>());
      L4Re::chksys(env->mem_alloc()->alloc(L4_PAGESIZE, ds.get(), 0));
      sz = ds->size();

      ASSERT_EQ(0, rm->attach(&start, sz, L4Re::Rm::Search_addr,
                              L4::Ipc::make_cap_rw(ds.get())));
    }

  good_pf(rm.get(), start, start, sz);
  good_pf(rm.get(), start + sz - 1, start, sz);

  L4::Cap<L4Re::Dataspace> oldds;
  ASSERT_EQ(0, rm->detach(start, &oldds));

  EXPECT_EQ(-1, bad_pf(rm.get(), start));
  EXPECT_EQ(-1, bad_pf(rm.get(), start + sz - 1));
}

/**
 * When a region manager is deleted its allocated memory is freed.
 *
 * The test allocates region managers until moe reports to be out of memory
 * and then deletes exactly one region manager. If the memory was freed correctly
 * it should be possible to allocate a new region manager.
 */
TEST_F(TestRm, ExhaustQuotaWithCreate)
{
  auto cap = create_fab(3 * L4_PAGESIZE);

  // Create dataspaces without deleting them until we are out of memory
  std::vector<L4Re::Util::Shared_cap<L4Re::Rm>> nslist;

  for (;;)
    {
      auto ns = make_shared_cap<L4Re::Rm>();

      long ret = l4_error(cap->create(ns.get()));
      if (ret == L4_EOK)
        nslist.push_back(ns);
      else
        {
          ASSERT_EQ(-L4_ENOMEM, ret);
          break;
        }
    }

  ASSERT_FALSE(nslist.empty());
  // free the previously allocated namespace
  nslist.pop_back();

  // after freeing, we should be able to allocate again
  auto ns = make_unique_del_cap<L4Re::Rm>();
  ASSERT_EQ(0, l4_error(cap->create(ns.get())));
}
