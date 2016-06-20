/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test dataspace implementation of moe.
 */

#include <climits>
#include <l4/re/dataspace>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>

#include <l4/atkins/tap/main>
#include <l4/atkins/debug>

#include "moe_helpers.h"

using std::tr1::tuple;
static Atkins::Dbg dbg{2};

struct TestGeneralDs : testing::Test {};

// Test for a given type of dataspace
struct TestDataspace : testing::Test
{
  TestDataspace(unsigned long flags, unsigned long size)
  : _flags(flags), _size(size) {}

  L4Re::Util::Auto_del_cap<L4Re::Dataspace>::Cap
  create_ds(unsigned long size = 0)
  {
    auto ds = make_auto_del_cap<L4Re::Dataspace>();
    L4Re::chksys(env->mem_alloc()->alloc(size ? size : _size, ds.get(), _flags));
    return ds;
  }

  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap
  make_ds_ro(L4::Cap<L4Re::Dataspace> ds)
  {
    auto ro_ds = make_auto_cap<L4Re::Dataspace>();
    env->task()->map(env->task(), ds.fpage(L4_FPAGE_RO), ro_ds.snd_base());
    return ro_ds;
  }

  unsigned long defsize() const
  { return _size; }

  unsigned long defflags() const
  { return _flags; }

private:
  unsigned long _flags;
  unsigned long _size;
};

// Need to test different dataspace sizes, as they have different
// implementations in moe (see moe/server/src/dataspace_noncont.cc).
const unsigned long DS_TESTSIZES[] = {
  L4_PAGESIZE >> 2,                                     // partial
  L4_PAGESIZE,                                          // Mem_one_page
  3 * L4_PAGESIZE,                                      // Mem_small
  L4_PAGESIZE * (L4_PAGESIZE/sizeof(unsigned long) + 1) // Mem_big
};

// Tests against normal dataspaces only
struct TestRegDs : TestDataspace, testing::WithParamInterface<unsigned long>
{
  TestRegDs() : TestDataspace(0, GetParam()) {}
};

static INSTANTIATE_TEST_CASE_P(RegDs, TestRegDs,
                               testing::ValuesIn(DS_TESTSIZES));

// Tests against continuous dataspaces only
struct TestContDs : TestDataspace
{
  TestContDs() : TestDataspace(L4Re::Mem_alloc::Continuous, 0) {}
};

// Tests against both types
struct TestAnyDs
: TestDataspace,
  testing::WithParamInterface<tuple<unsigned long, unsigned long> >
{
  TestAnyDs()
  : TestDataspace(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {}
};

static INSTANTIATE_TEST_CASE_P(
  All, TestAnyDs,
  testing::Combine(testing::Values(0, L4Re::Mem_alloc::Continuous),
//                                   L4Re::Mem_alloc::Continuous | L4Re::Mem_alloc::Super_pages ),
                   testing::ValuesIn(DS_TESTSIZES))
);

// for copy, need to test against any combination of source and dest
struct TestCrossDs
: TestDataspace,
  testing::WithParamInterface<tuple<unsigned long, unsigned long, unsigned long, unsigned long> >
{
  TestCrossDs()
  : TestDataspace(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {}

  L4Re::Util::Auto_del_cap<L4Re::Dataspace>::Cap
  create_src_ds(unsigned long size = 0)
  {
    auto ds = make_auto_del_cap<L4Re::Dataspace>();
    L4Re::chksys(env->mem_alloc()->alloc(size ? size : std::get<3>(GetParam()),
                                         ds.get(), std::get<2>(GetParam())));
    return ds;
  }

  unsigned long defsize_src() const
  { return std::get<3>(GetParam()); }
};

static INSTANTIATE_TEST_CASE_P(
  All, TestCrossDs,
  testing::Combine(testing::Values(0, L4Re::Mem_alloc::Continuous),
                   testing::ValuesIn(DS_TESTSIZES),
                   testing::Values(0, L4Re::Mem_alloc::Continuous),
                   testing::ValuesIn(DS_TESTSIZES))
);

/***********************************************************************
 * TESTS
 */

TEST_P(TestAnyDs, AllocateFull)
{
  auto ds = create_ds();
  EXPECT_EQ(0, ds->allocate(0, defsize()));
}

TEST_P(TestAnyDs, AllocateZeroSize)
{
  auto ds = create_ds();
  EXPECT_EQ(0, ds->allocate(0, 0));
}

TEST_P(TestAnyDs, AllocatePartial)
{
  auto ds = create_ds();
  if (defsize() <= L4_PAGESIZE)
    EXPECT_EQ(0, ds->allocate(defsize() / 2 - 1, 2));
  else
    EXPECT_EQ(0, ds->allocate(2 * L4_PAGESIZE - 1, 2));
}

TEST_P(TestRegDs, AllocateOutsideDs)
{
  auto ds = create_ds();
  EXPECT_EQ(-L4_ERANGE, ds->allocate(0, defsize() + 1));
  EXPECT_EQ(-L4_ERANGE, ds->allocate(defsize() + L4_PAGESIZE, 2));
}

TEST_P(TestRegDs, AllocateIllegal)
{
  auto ds = create_ds();
  EXPECT_EQ(-L4_ERANGE, ds->allocate(~0UL, 1));
  EXPECT_EQ(-L4_ERANGE, ds->allocate(0, ~0UL));
  EXPECT_EQ(-L4_ERANGE, ds->allocate(LONG_MAX - 1, LONG_MAX - 1));
}

TEST_P(TestAnyDs, ClearByte)
{
  auto ds = create_ds();

  L4Re::Rm::Auto_region<char *> r;
  ASSERT_EQ(0, env->rm()->attach(&r, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 ds.get(), 0));

  r.get()[100] = 'x';
  EXPECT_LE(0, ds->clear(100, 1));
  EXPECT_EQ('\0', r.get()[100]);
}

TEST_P(TestAnyDs, ClearUnallocated)
{
  auto ds = create_ds();
  EXPECT_LE(0, ds->clear(0, 100));
}

TEST_P(TestAnyDs, ClearEmpty)
{
  auto ds = create_ds();
  EXPECT_LE(0, ds->clear(10, 0));
}

TEST_P(TestAnyDs, ClearFull)
{
  auto ds = create_ds();
  L4Re::Rm::Auto_region<char *> reg;

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    {
      ASSERT_EQ(0, env->rm()->attach(&reg, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                     ds.get(), off));
      reg.get()[0] = 'x';
    }

  EXPECT_LE(0, ds->clear(0, defsize()));

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    {
      ASSERT_EQ(0, env->rm()->attach(&reg, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                     ds.get(), off));
      EXPECT_EQ('\0', reg.get()[0]);
    }
}

TEST_P(TestAnyDs, ClearOutsideRange)
{
  auto ds = create_ds();
  EXPECT_EQ(-L4_ERANGE, ds->clear(defsize() + L4_PAGESIZE, 1));
  EXPECT_EQ(-L4_ERANGE, ds->clear(~0UL, 1));
}

TEST_P(TestAnyDs, ClearOversized)
{
  auto ds = create_ds();
  EXPECT_LE(0, ds->clear(0, defsize() + L4_PAGESIZE));
  EXPECT_LE(0, ds->clear(defsize() - 1, 2 * L4_PAGESIZE));
  EXPECT_LE(0, ds->clear(0, ~0UL));
}

TEST_P(TestAnyDs, ClearBadRights)
{
  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());

  EXPECT_EQ(-L4_EACCESS, ro_ds->clear(0,1));
}


TEST_P(TestCrossDs, CopyInBytesFromNormal)
{
  const char* cmpstr = "Bindlestitch\n";

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  memset(srcptr.get(), 'x', strlen(cmpstr) + 10);
  strcpy(srcptr.get() + 2, cmpstr);

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), '!', defsize() < L4_PAGESIZE ? defsize() : L4_PAGESIZE);

  ASSERT_EQ(0, dest->copy_in(20, src.get(), 2, strlen(cmpstr) + 1));

  EXPECT_EQ(0, memcmp(destptr.get() + 20, cmpstr, strlen(cmpstr) + 1));
  EXPECT_EQ('!', destptr.get()[19]);
  EXPECT_NE('!', destptr.get()[20]);
  EXPECT_EQ('!', destptr.get()[20 + strlen(cmpstr) + 1]);
}

TEST_P(TestCrossDs, CopyInBytesFromUnallocated)
{
  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), 0, defsize() < L4_PAGESIZE ? defsize() : L4_PAGESIZE);

  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, 20));
}

TEST_P(TestAnyDs, CopyInBytesFromUnallocatedFull)
{
  auto src = create_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, defsize(), L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), 0, defsize());

  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, defsize()));
}

TEST_P(TestCrossDs, CopyInBytesToUnallocated)
{
  const char* teststr = "foobartuffy";
  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  strcpy(srcptr.get() + 2, teststr);

  ASSERT_EQ(0, dest->copy_in(0, src.get(), 0, 20));

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  EXPECT_EQ(0, strncmp(destptr.get() + 2, teststr, strlen(teststr) + 1));
}

TEST_P(TestAnyDs, CopyInBytesToUnallocatedFull)
{
  const char* teststr = "laif9reido0geij8yu2thaePhahloo1DahTogei8wopa8ahthe";
  auto src = create_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  strcpy(srcptr.get() + 100, teststr);

  ASSERT_EQ(0, dest->copy_in(0, src.get(), 0, defsize()));

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  EXPECT_EQ(0, strncmp(destptr.get() + 100, teststr, strlen(teststr) + 1));
}

TEST_P(TestCrossDs, CopyInBytesFromMultiplePages)
{
  if (defsize_src() < 2 * L4_PAGESIZE)
    return;

  const char* cmpstr = "ogjeogj943 G$03 a;s";

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, 2 * L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  memset(srcptr.get(), 'x', 2 * L4_PAGESIZE);
  strcpy(srcptr.get() + L4_PAGESIZE - 1, cmpstr);

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), '!', defsize() < L4_PAGESIZE ? defsize() : L4_PAGESIZE);

  ASSERT_EQ(0, dest->copy_in(2, src.get(), L4_PAGESIZE - 1, strlen(cmpstr)));

  EXPECT_EQ(0, memcmp(destptr.get() + 2, cmpstr, strlen(cmpstr)));
  EXPECT_EQ('!', destptr.get()[1]);
  EXPECT_EQ('!', destptr.get()[3 + strlen(cmpstr)]);
}

TEST_P(TestCrossDs, CopyInBytesToMultiplePages)
{
  if (defsize() < 2 * L4_PAGESIZE)
    return;

  const char* cmpstr = "leGom^g35/SflejGEG";

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Auto_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  memset(srcptr.get(), 'x',
         defsize_src() < L4_PAGESIZE ? defsize_src() : L4_PAGESIZE);
  strcpy(srcptr.get(), cmpstr);

  L4Re::Rm::Auto_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, 2 * L4_PAGESIZE,
                                 L4Re::Rm::Search_addr, dest.get(), 0));
  memset(destptr.get(), '!', 2 * L4_PAGESIZE);

  ASSERT_EQ(0, dest->copy_in(L4_PAGESIZE - 2, src.get(), 0, strlen(cmpstr)));

  EXPECT_EQ(0, memcmp(destptr.get() + L4_PAGESIZE - 2, cmpstr, strlen(cmpstr)));
  EXPECT_EQ('!', destptr.get()[L4_PAGESIZE - 3]);
  EXPECT_EQ('!', destptr.get()[L4_PAGESIZE + strlen(cmpstr) - 1]);
}

TEST_P(TestAnyDs, CopyInFully)
{
  auto src = create_ds();
  auto dest = create_ds();

  ASSERT_EQ(0, dest->copy_in(0, src.get(), 0, defsize()));
}

TEST_P(TestAnyDs, CopySelf)
{
  auto dest = create_ds();

  ASSERT_EQ(0, dest->copy_in(0, dest.get(), 0, defsize()));
}


TEST_P(TestAnyDs, CopyInEmpty)
{
  auto src = create_ds();
  auto dest = create_ds();

  ASSERT_EQ(0, dest->copy_in(defsize() - 1, src.get(), defsize() - 1, 0));
}

TEST_P(TestAnyDs, CopyInInvalidSrcCap)
{
  auto src = L4Re::chkcap(L4Re::Util::make_auto_cap<L4Re::Dataspace>());
  auto dest = create_ds();

  ASSERT_EQ(-L4_EINVAL, dest->copy_in(0, src.get(), 0, 100));
  ASSERT_EQ(-L4_EINVAL, dest->copy_in(0,
                                      L4::cap_reinterpret_cast<L4Re::Dataspace>(env->log()),
                                      0, 100));
}

TEST_P(TestAnyDs, CopyInDestOutOfBounds)
{
  auto src = create_ds();
  auto dest = create_ds();

  // Moe does not return any useful information here.
  // Just run this and hope all is well if it doesn't crash.
  EXPECT_EQ(0, dest->copy_in(defsize() + 1, src.get(), 0, 2));
  EXPECT_EQ(0, dest->copy_in(~0UL - 1, src.get(), 0, 1));
  EXPECT_EQ(0, dest->copy_in(defsize() - 2, src.get(), 0, 4));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, defsize() + 1));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, ~0UL));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), defsize() + 1, 2));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), defsize() - 2, 4));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), ~0UL - 1, 1));
}

TEST_P(TestCrossDs, CopyInBadRights)
{
  auto src = create_src_ds();
  auto dest = create_ds();
  auto ro_dest = make_ds_ro(dest.get());

  EXPECT_EQ(-L4_EACCESS, ro_dest->copy_in(0, src.get(), 0, defsize()));
}

TEST_P(TestAnyDs, Flags)
{
  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());

  long addflags = 0;
  if (!(defflags() & L4Re::Mem_alloc::Continuous))
    addflags |= 0x100;  // cow flag, well hidden away in the Moe server code

  EXPECT_EQ(L4Re::Dataspace::Map_rw | addflags, ds->flags());
  EXPECT_EQ(addflags, ro_ds->flags());
}

TEST_P(TestAnyDs, AllocatedIsEmptyRo)
{
  auto ds = create_ds();
  Fenced_auto_area reg(defsize());

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    EXPECT_EQ(0, ds->map(off, L4Re::Dataspace::Map_ro, reg.start() + off,
                         reg.start(), reg.end() - 1));

  EXPECT_TRUE(reg.check_fence());
  for (size_t i = 0; i < defsize(); ++i)
    EXPECT_EQ(0, reg.data<char>()[i]);
}

TEST_P(TestAnyDs, AllocatedIsEmptyRw)
{
  auto ds = create_ds();
  Fenced_auto_area reg(defsize());

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    EXPECT_EQ(0, ds->map(off, L4Re::Dataspace::Map_rw, reg.start() + off,
                         reg.start(), reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
  for (size_t i = 0; i < defsize(); ++i)
    EXPECT_EQ(0, reg.data<char>()[i]);
}


TEST_P(TestAnyDs, MapFull)
{
  auto ds = create_ds();
  Fenced_auto_area reg(defsize());

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    {
      EXPECT_EQ(0, ds->map(off, L4Re::Dataspace::Map_rw, reg.start() + off,
                           reg.start(), reg.end() - 1));
      // that should allow us to write on the mapped address
      reg.data<char>()[off] = 'x';
      EXPECT_TRUE(reg.check_fence());
    }
}

TEST_P(TestAnyDs, MapMinimal)
{
  auto ds = make_auto_cap<L4Re::Dataspace>();
  L4Re::chksys(env->mem_alloc()->alloc(defsize(), ds.get(), defflags()));
  Fenced_auto_area reg(defsize());

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    {
      EXPECT_EQ(0, ds->map(off, L4Re::Dataspace::Map_rw, reg.start() + off,
                           reg.start() + off, reg.start() + off));
      // that should allow us to write on the mapped address
      reg.data<char>()[off] = 'x';
      EXPECT_TRUE(reg.check_fence());
    }
}

TEST_F(TestGeneralDs, MapBadRegion)
{
  auto ds = create_ds();
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(0, ds->map(0, 0, reg.start(), reg.end(), reg.start()));
  EXPECT_TRUE(reg.check_fence());
  EXPECT_EQ(0, ds->map(100, 0, reg.start(), reg.end(), reg.start()));
  EXPECT_TRUE(reg.check_fence());
  EXPECT_EQ(0, ds->map(0, 0, reg.start(), reg.start(), ~0UL));
  EXPECT_TRUE(reg.check_fence());
  EXPECT_EQ(0, ds->map(0, 0, reg.start(), ~0UL, reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
}

TEST_F(TestGeneralDs, MapBadOffset)
{
  auto ds = create_ds(0, L4_PAGESIZE);
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(-L4_ERANGE, ds->map(L4_PAGESIZE + 2, 0, reg.start(),
                                reg.start(), reg.end()));
  EXPECT_TRUE(reg.check_fence());
}

TEST_P(TestAnyDs, MapInsufficientRights)
{
  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(-L4_EPERM, ro_ds->map(0, L4Re::Dataspace::Map_rw, reg.start(),
                                  reg.start(), reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
}

TEST_P(TestAnyDs, MapRegionFull)
{
  auto ds = create_ds();
  Fenced_auto_area reg(defsize());
  ASSERT_EQ(0, ds->map_region(0, L4Re::Dataspace::Map_rw,
                              reg.start(), reg.end()));
  EXPECT_TRUE(reg.check_fence());
  // check that we can write to the entire area
  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    reg.data<char>()[off] = 'x';
}

TEST_F(TestGeneralDs, MapRegionBadRegion)
{
  auto ds = create_ds();
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(0, ds->map_region(0, 0, reg.start(), ~0UL));
  EXPECT_TRUE(reg.check_fence());
  EXPECT_EQ(0, ds->map_region(0, 0, ~0UL, reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
  EXPECT_EQ(0, ds->map_region(0, 0, ~0UL, ~0UL));
  EXPECT_TRUE(reg.check_fence());
}

TEST_P(TestAnyDs, MapRegionInsufficientRights)
{
  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(-L4_EPERM, ro_ds->map_region(0, L4Re::Dataspace::Map_rw,
                                  reg.start(), reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
}


TEST_P(TestRegDs, Phys)
{
  auto ds = create_ds();
  l4_addr_t pa;
  l4_size_t sz;
  EXPECT_EQ(-L4_EINVAL, ds->phys(0, pa, sz));
}

TEST_F(TestContDs, Phys)
{
  auto ds = create_ds(1024);
  l4_addr_t pa;
  l4_size_t sz;
  EXPECT_EQ(0, ds->phys(0, pa, sz));
  EXPECT_LE(sz, 1024UL);
}

TEST_F(TestContDs, PhysUnaligned)
{
  auto ds = create_ds(1024);
  l4_addr_t pa;
  l4_size_t sz;

  EXPECT_EQ(0, ds->phys(123, pa, sz));
  EXPECT_EQ(0U, (pa - 123) % L4_PAGESIZE);
  EXPECT_EQ(1024U, (pa + sz) % L4_PAGESIZE);
}

TEST_P(TestAnyDs, Size)
{
  auto ds = create_ds();
  EXPECT_EQ(defsize(), ds->size());
}

TEST_F(TestGeneralDs, Size)
{
  auto ds = create_ds(0, 1);
  EXPECT_EQ(1UL, ds->size());
  ds = create_ds(0, INT_MAX - 1);
  EXPECT_EQ((unsigned long) INT_MAX - 1, ds->size());
}

TEST_P(TestRegDs, ExhaustQuotaMoeStructures)
{
  auto cap = create_ma(3 * L4_PAGESIZE);

  // Create dataspaces without deleting them until we are out of memory
  std::vector<L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap> dslist;

  for (;;)
    {
      auto ds = make_ref_cap<L4Re::Dataspace>();

      long ret = cap->alloc(defsize(), ds.get());
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
  auto ds = make_auto_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, cap->alloc(defsize(), ds.get()));
}
