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
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/sys/pager>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"

static Atkins::Dbg dbg{2};

struct TestRm : testing::Test
{
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

  L4Re::Util::Auto_del_cap<L4Re::Rm>::Cap
  create_blocked_rm()
  {
    auto rm = create_rm();

    L4Re::Rm::Region const *rl;
    l4_addr_t addr = 0;
    long n;
    while ((n = env->rm()->get_regions(addr, &rl)) > 0)
    {
      for (int i = 0; i < n; ++i)
      {
        auto const *r = &rl[i];
        l4_addr_t start = r->start;
        long ret = rm->reserve_area(&start, r->end - r->start);
        printf("Reserve: 0x%lx/0x%lx -> %lu\n", r->start, r->end, ret);
        addr = r->end + 1;
      }
    }

    l4_fpage_t utcbs = env->utcb_area();

    l4_addr_t start = l4_fpage_page(utcbs);
    rm->reserve_area(&start, l4_fpage_size(utcbs));

    return rm;
  }

};


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

TEST_F(TestRm, ReservePageFixed)
{
  auto rm = create_rm();

  unsigned long start = 0x1000000;
  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE));
  EXPECT_EQ(start, 0x1000000UL);
}

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

TEST_F(TestRm, ReserveUnaligned)
{
  auto rm = create_rm();

  unsigned long start = 0x1000010;
  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE));
  EXPECT_EQ(start, 0x1000010UL);

  start = 0x1000000;
  EXPECT_EQ(0, rm->reserve_area(&start, 10));
}

TEST_F(TestRm, ReserveTiny)
{
  auto rm = create_rm();

  unsigned long start = 0;
  EXPECT_EQ(0, rm->reserve_area(&start, 10, L4Re::Rm::Search_addr));
  EXPECT_EQ(0UL, start % L4_PAGESIZE);
  EXPECT_EQ(0, rm->reserve_area(&start, 10, L4Re::Rm::Search_addr, 1));
}

TEST_F(TestRm, ExhaustQuotaReserve)
{
  auto fab = L4Re::chkcap(L4Re::Util::make_auto_del_cap<L4::Factory>());
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

TEST_F(TestRm, FreeAreaSimple)
{
  auto rm = create_rm();

  unsigned long start = 0;
  EXPECT_EQ(0, rm->reserve_area(&start, L4_PAGESIZE, L4Re::Rm::Search_addr));
  EXPECT_NE(start, 0UL);

  EXPECT_EQ(0, rm->free_area(start));
}

TEST_F(TestRm, FreeAreaNonExisting)
{
  auto rm = create_rm();

  EXPECT_EQ(-L4_ENOENT, rm->free_area(100 * L4_PAGESIZE));
}

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

TEST_F(TestRm, AttachLooseDataspace)
{
  l4_addr_t start = 0;
  unsigned long sz;
  auto rm = create_blocked_rm();

    {
      // Cap should be removed without explicitly deleting.
      auto ds = L4Re::chkcap(L4Re::Util::make_auto_cap<L4Re::Dataspace>());
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


TEST_F(TestRm, ExhaustQuotaWithCreate)
{
  auto cap = create_fab(3 * L4_PAGESIZE);

  // Create dataspaces without deleting them until we are out of memory
  std::vector<L4Re::Util::Ref_cap<L4Re::Rm>::Cap> nslist;

  for (;;)
    {
      auto ns = make_ref_cap<L4Re::Rm>();

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
  auto ns = make_auto_del_cap<L4Re::Rm>();
  ASSERT_EQ(0, l4_error(cap->create(ns.get())));
}
