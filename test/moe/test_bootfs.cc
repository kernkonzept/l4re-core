/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test namespace implementation of moe.
 */

#include <cstring>

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/namespace>
#include <l4/re/error_helper>
#include <l4/sys/kdebug.h>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"

static const char *TESTFILE_CONTENT = "This is a test file.";

// Check if the boot fs is correctly exported
TEST(TestMoeEnv, BootFSIsExported)
{
  auto cap = env->get_cap<L4Re::Namespace>("rom");

  ASSERT_TRUE(cap.is_valid());
  ASSERT_LT(cap.cap() >> L4_CAP_SHIFT, env->first_free_cap());
}

// Test use of boot fs namespace
class TestMoeBootFs : public ::testing::Test
{
public:
  virtual void SetUp()
  {
    ns = L4Re::chkcap(env->get_cap<L4Re::Namespace>("rom"));
  }

  L4::Cap<L4Re::Namespace> ns;
};


TEST_F(TestMoeBootFs, QueryOurselves)
{
  auto cap = make_auto_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("test_bootfs", cap.get()));
  EXPECT_EQ(L4_EOK, ns->query("test_bootfsfoobar", 11, cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("test_bootfsfoobar", 12, cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("test_bootfs\0foobar", 18, cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("/test_bootfs", cap.get()));
  EXPECT_EQ(-L4_EBADPROTO, ns->query("test_bootfs/something", cap.get()));
  EXPECT_LT(0,
            ns->query("test_bootfs/something", cap.get(),
                      L4Re::Namespace::To_default, 0, false));
}

TEST_F(TestMoeBootFs, QueryAllowedModules)
{
  auto cap = make_auto_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("l4re", cap.get()));
  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));

  L4::Cap<L4Re::Dataspace> c(cap.get().cap());
  EXPECT_EQ(L4Re::Dataspace::Map_ro, c->flags() & 1);
}

TEST_F(TestMoeBootFs, QueryForbiddenModules)
{
  auto cap = make_auto_cap<void>();

  EXPECT_EQ(-L4_ENOENT, ns->query("fiasco", cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("kernel", cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("moe", cap.get()));
}

TEST_F(TestMoeBootFs, RegisterDeleteEntry)
{
  auto cap = make_auto_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(L4_EOK, ns->register_obj("foo", cap.get()));
  EXPECT_EQ(L4_EOK, ns->query("foo", cap.get()));
  EXPECT_EQ(L4_EOK, ns->unlink("foo"));
}

TEST_F(TestMoeBootFs, MapRomSpace)
{
  auto ds = make_auto_cap<L4Re::Dataspace>();

  ASSERT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", ds.get()));
  size_t sz = ds->size();
  ASSERT_GT(sz, strlen(TESTFILE_CONTENT));

  L4Re::Rm::Auto_region<char *> reg;
  ASSERT_EQ(L4_EOK, env->rm()->attach(&reg, sz, L4Re::Rm::Search_addr,
                                      ds.get(), 0, L4_PAGESHIFT));
  ASSERT_EQ(0, memcmp(reg.get(), TESTFILE_CONTENT,
                      strlen(TESTFILE_CONTENT)));

  for (unsigned i = strlen(TESTFILE_CONTENT) + 1; i < L4_PAGESIZE; ++i)
    EXPECT_EQ(0, reg.get()[i]);
}

TEST_F(TestMoeBootFs, FailToDeleteRomCapability)
{
  auto cap = make_auto_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  ASSERT_EQ(L4_EOK, l4_error(env->task()->unmap(cap.fpage(), L4_FP_DELETE_OBJ |
                                                             L4_FP_ALL_SPACES)));
  // Cap is gone in our task.
  ASSERT_LE(cap.validate(L4_BASE_TASK_CAP).label(), 0);
  // But we can still get it back, so delete failed.
  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
}

TEST_F(TestMoeBootFs, FailToDeleteRomCapabilityWhenRemapping)
{
  auto cap = make_auto_cap<void>();
  auto newns = create_ns();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  // register it with the new namespace
  ASSERT_EQ(L4_EOK, newns->register_obj("new", cap.get()));
  // now get that cap from the new namespace
  auto ds = make_auto_cap<L4Re::Dataspace>();
  ASSERT_EQ(L4_EOK, newns->query("new", ds.get()));

  // unmapping still shouldn't have an effect
  ASSERT_EQ(L4_EOK, l4_error(env->task()->unmap(ds.fpage(), L4_FP_DELETE_OBJ |
                                                             L4_FP_ALL_SPACES)));
  // Cap is gone in our task.
  ASSERT_LE(ds.validate(L4_BASE_TASK_CAP).label(), 0);
  // But we can still get it back, so delete failed.
  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  ASSERT_GT(cap.validate(L4_BASE_TASK_CAP).label(), 0);
}

TEST_F(TestMoeBootFs, FailToDeleteInitialEntry)
{
  EXPECT_EQ(-L4_EACCESS, ns->unlink("moe_bootfs_example.txt"));
}

TEST_F(TestMoeBootFs, FailToOverwriteEntry)
{
  auto cap = create_ds();

  EXPECT_EQ(-L4_EEXIST, ns->register_obj("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(-L4_EEXIST, ns->register_obj("moe_bootfs_example.txt", cap.get(),
                                         L4Re::Namespace::Overwrite));
}

TEST_F(TestMoeBootFs, FailToClearDataspace)
{
  auto cap = make_auto_cap<L4Re::Dataspace>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(-L4_EACCESS, cap->clear(0, 10));
}

TEST_F(TestMoeBootFs, FailToAllocateDataspace)
{
  auto cap = make_auto_cap<L4Re::Dataspace>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(0, cap->allocate(0, 10));

  // check that nothing was deleted
  L4Re::Rm::Auto_region<char *> reg;
  ASSERT_EQ(L4_EOK, env->rm()->attach(&reg, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                      cap.get(), 0));
  ASSERT_EQ(0, memcmp(reg.get(), TESTFILE_CONTENT, strlen(TESTFILE_CONTENT)));

}

