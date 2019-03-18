/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test dataspace implementation of moe.
 *
 * \note When test descriptions refer to 'unallocated dataspaces' they mean
 *       dataspaces that have neither been explicitly allocated yet
 *       (via allocate()) nor have they been mapped and therefore are not
 *       backed with physical memory yet.
 */

#include <climits>
#include <l4/re/dataspace>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/util/dataspace_svr>

#include <l4/atkins/tap/main>
#include <l4/atkins/debug>
#include <l4/atkins/l4_assert>

#include "moe_helpers.h"

using std::tuple;
static Atkins::Dbg dbg{2};

struct TestGeneralDs : testing::Test {};

/**
 *  Test for a given type of dataspace.
 */
struct TestDataspace : testing::Test
{
  TestDataspace(unsigned long flags, unsigned long size)
  : _flags(flags), _size(size) {}

  L4Re::Util::Unique_del_cap<L4Re::Dataspace>
  create_ds(unsigned long size = 0)
  {
    auto ds = make_unique_del_cap<L4Re::Dataspace>();
    L4Re::chksys(env->mem_alloc()->alloc(size ? size : _size, ds.get(), _flags));
    return ds;
  }

  L4Re::Util::Unique_cap<L4Re::Dataspace>
  make_ds_ro(L4::Cap<L4Re::Dataspace> ds)
  {
    auto ro_ds = make_unique_cap<L4Re::Dataspace>();
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

/**
 * Different dataspace sizes to test.
 *
 * These are needed because they have different implementations in moe.
 *
 * \see moe/server/src/dataspace_noncont.cc
 */
const unsigned long DS_TESTSIZES[] = {
  L4_PAGESIZE >> 2,                                     // partial
  L4_PAGESIZE,                                          // Mem_one_page
  3 * L4_PAGESIZE,                                      // Mem_small
  L4_PAGESIZE * (L4_PAGESIZE/sizeof(unsigned long) + 1) // Mem_big
};

/**
 * Tests against normal dataspaces only.
 */
struct TestRegDs : TestDataspace, testing::WithParamInterface<unsigned long>
{
  TestRegDs() : TestDataspace(0, GetParam()) {}
};

static INSTANTIATE_TEST_CASE_P(RegDs, TestRegDs,
                               testing::ValuesIn(DS_TESTSIZES));

/**
 * Tests against continuous dataspaces only.
 */
struct TestContDs : TestDataspace
{
  TestContDs() : TestDataspace(L4Re::Mem_alloc::Continuous, 0) {}
};

/**
 * Tests against both types of dataspaces.
 */
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

/**
 * Tests for coping between dataspaces.
 *
 * This needs to test against any combination of source and destination.
 */
struct TestCrossDs
: TestDataspace,
  testing::WithParamInterface<tuple<unsigned long, unsigned long, unsigned long, unsigned long> >
{
  TestCrossDs()
  : TestDataspace(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {}

  L4Re::Util::Unique_del_cap<L4Re::Dataspace>
  create_src_ds(unsigned long size = 0)
  {
    auto ds = make_unique_del_cap<L4Re::Dataspace>();
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

/* **********************************************************************
 * TESTS
 */

/**
 * A dataspace can be allocated all at once.
 *
 * \see L4Re::Dataspace.allocate
 */
TEST_P(TestAnyDs, AllocateFull)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.allocate");

  auto ds = create_ds();
  EXPECT_EQ(0, ds->allocate(0, defsize()));
}

/**
 * Allocating 0 bytes of memory will be ignored.
 *
 * \see L4Re::Dataspace.allocate
 */
TEST_P(TestAnyDs, AllocateZeroSize)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.allocate");

  auto ds = create_ds();
  EXPECT_EQ(0, ds->allocate(0, 0));
}

/**
 * A dataspace can be partially allocated.
 *
 * \see L4Re::Dataspace.allocate
 */
TEST_P(TestAnyDs, AllocatePartial)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.allocate");

  auto ds = create_ds();
  if (defsize() <= L4_PAGESIZE)
    EXPECT_EQ(0, ds->allocate(defsize() / 2 - 1, 2));
  else
    EXPECT_EQ(0, ds->allocate(2 * L4_PAGESIZE - 1, 2));
}

/**
 * Allocating memory outside the dataspace fails.
 *
 * \see L4Re::Dataspace.allocate
 */
TEST_P(TestRegDs, AllocateOutsideDs)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.allocate");

  auto ds = create_ds();
  EXPECT_EQ(-L4_ERANGE, ds->allocate(0, defsize() + 1));
  EXPECT_EQ(-L4_ERANGE, ds->allocate(defsize() + L4_PAGESIZE, 2));
}

/**
 * Calling allocate with illegal values fails.
 *
 * \see L4Re::Dataspace.allocate
 */
TEST_P(TestRegDs, AllocateIllegal)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.allocate");

  auto ds = create_ds();
  EXPECT_EQ(-L4_ERANGE, ds->allocate(~0UL, 1));
  EXPECT_EQ(-L4_ERANGE, ds->allocate(0, ~0UL));
  EXPECT_EQ(-L4_ERANGE, ds->allocate(LONG_MAX - 1, LONG_MAX - 1));
}

/**
 * A single byte in a dataspace can be cleared.
 *
 * \see L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearByte)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();

  L4Re::Rm::Unique_region<char *> r;
  ASSERT_EQ(0, env->rm()->attach(&r, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 ds.get(), 0));

  r.get()[100] = 'x';
  EXPECT_LE(0, ds->clear(100, 1));
  EXPECT_EQ('\0', r.get()[100]);
}

/**
 * Dataspace memory cannot be cleared when it has not been allocated yet.
 *
 * \see L4Re::Dataspace.allocate, L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearUnallocated)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();
  EXPECT_LE(0, ds->clear(0, 100));
}

/**
 * Clearing 0 bytes of memory fails.
 *
 * \see L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearEmpty)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();
  EXPECT_LE(0, ds->clear(10, 0));
}

/**
 * Clearing the complete dataspace sets the memory to 0.
 *
 * \see L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearFull)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();
  L4Re::Rm::Unique_region<char *> reg;

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

/**
 * Clearing memory outside the dataspace fails.
 *
 * \see L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearOutsideRange)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();
  EXPECT_EQ(-L4_ERANGE, ds->clear(defsize() + L4_PAGESIZE, 1));
  EXPECT_EQ(-L4_ERANGE, ds->clear(~0UL, 1));
}

/**
 * Clearing memory partially outside the dataspace fails.
 *
 * \see L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearOversized)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();
  EXPECT_LE(0, ds->clear(0, defsize() + L4_PAGESIZE));
  EXPECT_LE(0, ds->clear(defsize() - 1, 2 * L4_PAGESIZE));
  EXPECT_LE(0, ds->clear(0, ~0UL));
}

/**
 * Clearing memory in the dataspace requires write rights.
 *
 * \see L4Re::Dataspace.clear
 */
TEST_P(TestAnyDs, ClearBadRights)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.clear");

  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());

  EXPECT_EQ(-L4_EACCESS, ro_ds->clear(0,1));
}

/**
 * Content can be copied from a dataspace with allocated memory.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestCrossDs, CopyInBytesFromNormal)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  const char* cmpstr = "Bindlestitch\n";

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  memset(srcptr.get(), 'x', strlen(cmpstr) + 10);
  strcpy(srcptr.get() + 2, cmpstr);

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), '!', defsize() < L4_PAGESIZE ? defsize() : L4_PAGESIZE);

  ASSERT_EQ(0, dest->copy_in(20, src.get(), 2, strlen(cmpstr) + 1));

  EXPECT_EQ(0, memcmp(destptr.get() + 20, cmpstr, strlen(cmpstr) + 1));
  EXPECT_EQ('!', destptr.get()[19]);
  EXPECT_NE('!', destptr.get()[20]);
  EXPECT_EQ('!', destptr.get()[20 + strlen(cmpstr) + 1]);
}

/**
 * Content can be partially copied from an unallocated dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestCrossDs, CopyInBytesFromUnallocated)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), 0, defsize() < L4_PAGESIZE ? defsize() : L4_PAGESIZE);

  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, 20));
}

/**
 * Content can be fully copied from an unallocated dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopyInBytesFromUnallocatedFull)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = create_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, defsize(), L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), 0, defsize());

  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, defsize()));
}

/**
 * Content can be partially copied to an unallocated dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestCrossDs, CopyInBytesToUnallocated)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  const char* teststr = "foobartuffy";
  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  strcpy(srcptr.get() + 2, teststr);

  ASSERT_EQ(0, dest->copy_in(0, src.get(), 0, 20));

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  EXPECT_EQ(0, strncmp(destptr.get() + 2, teststr, strlen(teststr) + 1));
}

/**
 * Content can be fully copied to an unallocated dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopyInBytesToUnallocatedFull)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  const char* teststr = "laif9reido0geij8yu2thaePhahloo1DahTogei8wopa8ahthe";
  auto src = create_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  strcpy(srcptr.get() + 100, teststr);

  ASSERT_EQ(0, dest->copy_in(0, src.get(), 0, defsize()));

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  EXPECT_EQ(0, strncmp(destptr.get() + 100, teststr, strlen(teststr) + 1));
}

/**
 * Copying content from one allocated dataspace to another works over
 * hardware page boundaries in the source dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestCrossDs, CopyInBytesFromMultiplePages)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  if (defsize_src() < 2 * L4_PAGESIZE)
    return;

  const char* cmpstr = "ogjeogj943 G$03 a;s";

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, 2 * L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  memset(srcptr.get(), 'x', 2 * L4_PAGESIZE);
  strcpy(srcptr.get() + L4_PAGESIZE - 1, cmpstr);

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 dest.get(), 0));
  memset(destptr.get(), '!', defsize() < L4_PAGESIZE ? defsize() : L4_PAGESIZE);

  ASSERT_EQ(0, dest->copy_in(2, src.get(), L4_PAGESIZE - 1, strlen(cmpstr)));

  EXPECT_EQ(0, memcmp(destptr.get() + 2, cmpstr, strlen(cmpstr)));
  EXPECT_EQ('!', destptr.get()[1]);
  EXPECT_EQ('!', destptr.get()[3 + strlen(cmpstr)]);
}

/**
 * Copying content from one allocated dataspace to another works over
 * hardware page boundaries in the destination dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestCrossDs, CopyInBytesToMultiplePages)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  if (defsize() < 2 * L4_PAGESIZE)
    return;

  const char* cmpstr = "leGom^g35/SflejGEG";

  auto src = create_src_ds();
  auto dest = create_ds();

  L4Re::Rm::Unique_region<char *> srcptr;
  ASSERT_EQ(0, env->rm()->attach(&srcptr, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                 src.get(), 0));
  memset(srcptr.get(), 'x',
         defsize_src() < L4_PAGESIZE ? defsize_src() : L4_PAGESIZE);
  strcpy(srcptr.get(), cmpstr);

  L4Re::Rm::Unique_region<char *> destptr;
  ASSERT_EQ(0, env->rm()->attach(&destptr, 2 * L4_PAGESIZE,
                                 L4Re::Rm::Search_addr, dest.get(), 0));
  memset(destptr.get(), '!', 2 * L4_PAGESIZE);

  ASSERT_EQ(0, dest->copy_in(L4_PAGESIZE - 2, src.get(), 0, strlen(cmpstr)));

  EXPECT_EQ(0, memcmp(destptr.get() + L4_PAGESIZE - 2, cmpstr, strlen(cmpstr)));
  EXPECT_EQ('!', destptr.get()[L4_PAGESIZE - 3]);
  EXPECT_EQ('!', destptr.get()[L4_PAGESIZE + strlen(cmpstr) - 1]);
}

/**
 * Content can be fully copied from one unallocated dataspace to another
 * unallocated dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopyInFully)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = create_ds();
  auto dest = create_ds();

  ASSERT_EQ(0, dest->copy_in(0, src.get(), 0, defsize()));
}

/**
 * Content can be copied into the same dataspace.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopySelf)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto dest = create_ds();

  ASSERT_EQ(0, dest->copy_in(0, dest.get(), 0, defsize()));
}

/**
 * Content of 0 byte size can be copied.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopyInEmpty)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = create_ds();
  auto dest = create_ds();

  ASSERT_EQ(0, dest->copy_in(defsize() - 1, src.get(), defsize() - 1, 0));
}

/**
 * Copying from sources that are not moe dataspaces fails.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopyInInvalidSrcCap)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = L4Re::chkcap(L4Re::Util::make_unique_cap<L4Re::Dataspace>());
  auto dest = create_ds();

  ASSERT_EQ(-L4_EINVAL, dest->copy_in(0, src.get(), 0, 100));
  ASSERT_EQ(-L4_EINVAL, dest->copy_in(0,
                                      L4::cap_reinterpret_cast<L4Re::Dataspace>(env->log()),
                                      0, 100));
}

/**
 * Copying content from outside the dataspace does not crash the
 * system.
 *
 * The behaviour of copy_in for areas outside the dataspaces is undefined
 * and Moe chooses not return any useful information here. So just make sure
 * these calls all pass without crashing Moe.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestAnyDs, CopyInDestOutOfBounds)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = create_ds();
  auto dest = create_ds();

  EXPECT_EQ(0, dest->copy_in(defsize() + 1, src.get(), 0, 2));
  EXPECT_EQ(0, dest->copy_in(~0UL - 1, src.get(), 0, 1));
  EXPECT_EQ(0, dest->copy_in(defsize() - 2, src.get(), 0, 4));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, defsize() + 1));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), 0, ~0UL));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), defsize() + 1, 2));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), defsize() - 2, 4));
  EXPECT_EQ(0, dest->copy_in(0, src.get(), ~0UL - 1, 1));
}

/**
 * Copying into a read-only dataspace fails.
 *
 * \see L4Re::Dataspace.copy_in
 */
TEST_P(TestCrossDs, CopyInBadRights)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.copy_in");

  auto src = create_src_ds();
  auto dest = create_ds();
  auto ro_dest = make_ds_ro(dest.get());

  EXPECT_EQ(-L4_EACCESS, ro_dest->copy_in(0, src.get(), 0, defsize()));
}

/**
 * Normal dataspaces report to be copy-on-write, while all other
 * types of moe dataspaces are not copy-on-write capable.
 *
 * \see L4Re::Dataspace.flags
 */
TEST_P(TestAnyDs, Flags)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.flags");

  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());

  EXPECT_EQ(L4Re::Dataspace::Map_rwx,
            ds->flags() & L4Re::Dataspace::Map_flags_mask);
  EXPECT_EQ(L4Re::Dataspace::Map_rx,
            ro_ds->flags() & L4Re::Dataspace::Map_flags_mask);
}

/**
 * A freshly created dataspace can be partially mapped read-only and
 * contains 0 content.
 *
 * \see L4Re->Dataspace.map
 */
TEST_P(TestAnyDs, AllocatedIsEmptyRo)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

  auto ds = create_ds();
  Fenced_auto_area reg(defsize());

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    EXPECT_EQ(0, ds->map(off, L4Re::Dataspace::Map_ro, reg.start() + off,
                         reg.start(), reg.end() - 1));

  EXPECT_TRUE(reg.check_fence());
  for (size_t i = 0; i < defsize(); ++i)
    EXPECT_EQ(0, reg.data<char>()[i]);
}

/**
 * A freshly created dataspace can be partially mapped read-write and
 * contains 0 content.
 *
 * \see L4Re->Dataspace.map
 */
TEST_P(TestAnyDs, AllocatedIsEmptyRw)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

  auto ds = create_ds();
  Fenced_auto_area reg(defsize());

  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    EXPECT_EQ(0, ds->map(off, L4Re::Dataspace::Map_rw, reg.start() + off,
                         reg.start(), reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
  for (size_t i = 0; i < defsize(); ++i)
    EXPECT_EQ(0, reg.data<char>()[i]);
}

/**
 * The freshly created dataspace can be mapped read-write and be written to.
 *
 * \see L4Re->Dataspace.map
 */
TEST_P(TestAnyDs, MapFull)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

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

/**
 * Every single page of a dataspace can be mapped read-write
 * and be written to.
 *
 * \see L4Re->Dataspace.map
 */
TEST_P(TestAnyDs, MapMinimal)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

  auto ds = make_unique_cap<L4Re::Dataspace>();
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

/**
 * Bad parameters for mapping are ignored.
 *
 * \see L4Re->Dataspace.map
 */
TEST_F(TestGeneralDs, MapBadRegion)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

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

/**
 * Mapping from outside the dataspace fails.
 *
 * \see L4Re->Dataspace.map
 */
TEST_F(TestGeneralDs, MapBadOffset)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

  auto ds = create_ds(0, L4_PAGESIZE);
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(-L4_ERANGE, ds->map(L4_PAGESIZE + 2, 0, reg.start(),
                                reg.start(), reg.end()));
  EXPECT_TRUE(reg.check_fence());
}

/**
 * Dataspaces cannot be mapped with write rights when the dataspace
 * capability is read-only.
 *
 * \see L4Re->Dataspace.map
 */
TEST_P(TestAnyDs, MapInsufficientRights)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map");

  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(-L4_EPERM, ro_ds->map(0, L4Re::Dataspace::Map_rw, reg.start(),
                                  reg.start(), reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
}

/**
 * A dataspace can be mapped fully with map_region().
 *
 * \see L4Re->Dataspace.map_region
 */
TEST_P(TestAnyDs, MapRegionFull)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map_region");

  auto ds = create_ds();
  Fenced_auto_area reg(defsize());
  ASSERT_EQ(0, ds->map_region(0, L4Re::Dataspace::Map_rw,
                              reg.start(), reg.end()));
  EXPECT_TRUE(reg.check_fence());
  // check that we can write to the entire area
  for (l4_addr_t off = 0; off < defsize(); off += L4_PAGESIZE)
    reg.data<char>()[off] = 'x';
}

/**
 * map_region() returns an error when the target area is larger than
 * the dataspace.
 *
 * \see L4Re->Dataspace.map_region
 */
TEST_F(TestGeneralDs, MapRegionTargetRegionTooLarge)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map_region");

  auto ds = create_ds(0, L4_PAGESIZE);
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_L4ERR(L4_ERANGE, ds->map_region(0, L4Re::Dataspace::Map_rw,
                                         reg.start(), reg.end() + L4_PAGESIZE))
    << "Request mapping into an area that is twice as big as the datapsace.";
  EXPECT_TRUE(reg.check_fence())
    << "The memory around the target area is untouched.";
}

/**
 * When map_region() receives a maximum address that is smaller or
 * equal to the minimum address, then nothing happens.
 *
 * \see L4Re->Dataspace.map_region
 */
TEST_F(TestGeneralDs, MapRegionMaxAddrSmallerMinAddr)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map_region");

  auto ds = create_ds(0, L4_PAGESIZE);
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_L4OK(ds->map_region(0, L4Re::Dataspace::Map_rw,
                             reg.start(), reg.start()))
    << "Request mapping into an area where start and end address are the same.";
  EXPECT_TRUE(reg.check_fence())
    << "The memory around the target area is untouched.";

  EXPECT_L4OK(ds->map_region(0, L4Re::Dataspace::Map_rw,
                             reg.end(), reg.start()))
    << "Request mapping into an area with max_addr < min_addr.";
  EXPECT_TRUE(reg.check_fence())
    << "The memory around the target area is still untouched.";
}

/**
 * Dataspaces cannot be mapped with map_region() with write rights when the
 * dataspace capability is read-only.
 *
 * \see L4Re->Dataspace.map_region
 */
TEST_P(TestAnyDs, MapRegionInsufficientRights)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.map_region");

  auto ds = create_ds();
  auto ro_ds = make_ds_ro(ds.get());
  Fenced_auto_area reg(L4_PAGESIZE);

  EXPECT_EQ(-L4_EPERM, ro_ds->map_region(0, L4Re::Dataspace::Map_rw,
                                  reg.start(), reg.end() - 1));
  EXPECT_TRUE(reg.check_fence());
}

/**
 * The address of the underlying physical address is not available
 * for regular dataspaces.
 *
 * \see L4Re->Dataspace.phys
 */
TEST_P(TestRegDs, Phys)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.phys");

  auto ds = create_ds();
  l4_addr_t pa;
  l4_size_t sz;
  EXPECT_EQ(-L4_EINVAL, ds->phys(0, pa, sz));
}

/**
 * Continuous dataspaces have a physical address available and the
 * underlying physical memory covers the entire dataspace.
 *
 * \see L4Re->Dataspace.phys
 */
TEST_F(TestContDs, Phys)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.phys");

  auto ds = create_ds(1024);
  l4_addr_t pa;
  l4_size_t sz;
  EXPECT_EQ(0, ds->phys(0, pa, sz));
  EXPECT_LE(sz, 1024UL);
}

/**
 * When requesting the physical address of an arbitrary offset in
 * the dataspace, the returned physical address has the same page offset.
 *
 * \see L4Re::Dataspace.phys
 */
TEST_F(TestContDs, PhysUnaligned)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.phys");

  auto ds = create_ds(1024);
  l4_addr_t pa;
  l4_size_t sz;

  EXPECT_EQ(0, ds->phys(123, pa, sz));
  EXPECT_EQ(0U, (pa - 123) % L4_PAGESIZE);
  EXPECT_EQ(1024U, (pa + sz) % L4_PAGESIZE);
}

/**
 * The dataspace reports the same size as was requested when creating
 * the dataspace.
 *
 * \see L4Re::Dataspace.size
 */
TEST_P(TestAnyDs, Size)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.size");

  auto ds = create_ds();
  EXPECT_EQ(defsize(), ds->size());
}

/**
 * When creating a dataspace with a size not rounded to a page size
 * the unaligned size is reported.
 *
 * \see L4Re::Dataspace.size
 */
TEST_F(TestGeneralDs, UnalignedSize)
{
  TAP_COMP_FUNC("Moe", "L4Re::Dataspace.size");

  auto ds = create_ds(0, 1);
  EXPECT_EQ(1UL, ds->size());
  ds = create_ds(0, INT_MAX - 1);
  EXPECT_EQ((unsigned long) INT_MAX - 1, ds->size());
}

/**
 * Dataspace memory can only be created up to the quota given
 * in the dataspace's factory.
 *
 * \see L4Re::Mem_alloc.alloc
 */
TEST_P(TestRegDs, ExhaustQuotaMoeStructures)
{
  TAP_COMP_FUNC("Moe", "L4Re::Mem_alloc.alloc");

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
  auto ds = make_unique_cap<L4Re::Dataspace>();
  ASSERT_EQ(0, cap->alloc(defsize(), ds.get()));
}
