/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test the initial name spaces exported by moe to the initial task.
 *
 * Note that tests only check the content and behaviour of the rom
 * name space, there are extra tests for name spaces in general.
 */

#include <cstring>

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/namespace>
#include <l4/re/error_helper>
#include <l4/sys/kdebug.h>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"

static const char *TESTFILE_CONTENT = "This is a test file.";

/**
 * Moe exports the initial namespace via the rom capability.
 */
TEST(TestMoeEnv, BootFSIsExported)
{
  auto cap = env->get_cap<L4Re::Namespace>("rom");

  ASSERT_TRUE(cap.is_valid());
  ASSERT_LT(cap.cap() >> L4_CAP_SHIFT, env->first_free_cap());
}

class TestMoeBootFs : public ::testing::Test
{
public:
  virtual void SetUp()
  {
    ns = L4Re::chkcap(env->get_cap<L4Re::Namespace>("rom"));
  }

  L4::Cap<L4Re::Namespace> ns;
};

/**
 * When querying the namespace, special characters and string length
 * are handled correctly.
 */
TEST_F(TestMoeBootFs, QueryOurselves)
{
  auto cap = make_unique_cap<void>();

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

/**
 * All modules in the rom namespace can be found.
 */
TEST_F(TestMoeBootFs, QueryAllowedModules)
{
  auto cap = make_unique_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("l4re", cap.get()));
  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));

  L4::Cap<L4Re::Dataspace> c(cap.get().cap());
  EXPECT_EQ(L4Re::Dataspace::Map_ro, c->flags() & 1);
}

/**
 * Boot modules that have not been exported into the test's rom namespace
 * cannot be found.
 */
TEST_F(TestMoeBootFs, QueryForbiddenModules)
{
  auto cap = make_unique_cap<void>();

  EXPECT_EQ(-L4_ENOENT, ns->query("fiasco", cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("kernel", cap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("moe", cap.get()));
}

/**
 * New entries can be registered and deleted in the rom namespace.
 */
TEST_F(TestMoeBootFs, RegisterDeleteEntry)
{
  auto cap = make_unique_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(L4_EOK, ns->register_obj("foo", cap.get()));
  EXPECT_EQ(L4_EOK, ns->query("foo", cap.get()));
  EXPECT_EQ(L4_EOK, ns->unlink("foo"));
}

/**
 * When querying the rom namespace, the returned capability is a
 * dataspace whose content can be mapped into the test task and read.
 */
TEST_F(TestMoeBootFs, MapRomSpace)
{
  auto ds = make_unique_cap<L4Re::Dataspace>();

  ASSERT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", ds.get()))
    << "Query rom namespace for test file";
  size_t sz = ds->size();
  ASSERT_GT(sz, strlen(TESTFILE_CONTENT))
    << "The returned dataspace has the expected content size.";

  L4Re::Rm::Unique_region<char *> reg;
  ASSERT_EQ(L4_EOK, env->rm()->attach(&reg, sz, L4Re::Rm::Search_addr,
                                      ds.get(), 0, L4_PAGESHIFT))
    << "Attach the dataspace locally.";
  ASSERT_EQ(0, memcmp(reg.get(), TESTFILE_CONTENT,
                      strlen(TESTFILE_CONTENT)))
    << "Reading from the attached memory yields the expected content.";

  // the reminder of the file is 0
  for (unsigned i = strlen(TESTFILE_CONTENT) + 1; i < L4_PAGESIZE; ++i)
    EXPECT_EQ(0, reg.get()[i]);
}

/**
 * Capabilities in the rom namespace are mapped without the delete right.
 */
TEST_F(TestMoeBootFs, FailToDeleteRomCapability)
{
  auto cap = make_unique_cap<void>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()))
    << "Query rom namespace for the test file";
  ASSERT_EQ(L4_EOK, l4_error(env->task()->unmap(cap.fpage(), L4_FP_DELETE_OBJ |
                                                             L4_FP_ALL_SPACES)))
    << "Unmap the received capability with delete.";
  ASSERT_LE(cap.validate(L4_BASE_TASK_CAP).label(), 0)
    << "The capability is gone in the test task.";
  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()))
    << "But querying again works.";
  ASSERT_GT(cap.validate(L4_BASE_TASK_CAP).label(), 0)
    << "And a valid capability is returned, so delete failed.";
}

/**
 * Capabilities cannot gain delete right when registering them
 * in a new namespace.
 */
TEST_F(TestMoeBootFs, FailToDeleteRomCapabilityWhenRemapping)
{
  auto cap = make_unique_cap<void>();
  auto newns = create_ns();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()))
    << "Query rom namespace for the test file";
  ASSERT_EQ(L4_EOK, newns->register_obj("new", cap.get()))
    << "Register the received capability with the new namespace.";
  auto ds = make_unique_cap<L4Re::Dataspace>();
  ASSERT_EQ(L4_EOK, newns->query("new", ds.get()))
    << "Get a capability back for the newly registered name.";

  ASSERT_EQ(L4_EOK, l4_error(env->task()->unmap(ds.fpage(), L4_FP_DELETE_OBJ |
                                                             L4_FP_ALL_SPACES)))
    << "Unmap the newly received capability.";
  ASSERT_LE(ds.validate(L4_BASE_TASK_CAP).label(), 0)
    << "The capability is gone in the test task.";
  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()))
    << "But querying again works.";
  ASSERT_GT(cap.validate(L4_BASE_TASK_CAP).label(), 0)
    << "And a valid capability is returned, so delete failed.";
}

/**
 * The initial set of modules cannot be deleted from the rom namespace.
 */
TEST_F(TestMoeBootFs, FailToDeleteInitialEntry)
{
  EXPECT_EQ(-L4_EACCESS, ns->unlink("moe_bootfs_example.txt"));
}

/**
 * The initial set of modules cannot be overwritten in the rom namespace.
 */
TEST_F(TestMoeBootFs, FailToOverwriteEntry)
{
  auto cap = create_ds();

  EXPECT_EQ(-L4_EEXIST, ns->register_obj("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(-L4_EEXIST, ns->register_obj("moe_bootfs_example.txt", cap.get(),
                                         L4Re::Namespace::Overwrite));
}

/**
 * The initial set of modules in the rom namespace is read-only.
 *
 * This is tested by trying to clear the content of the dataspace.
 */
TEST_F(TestMoeBootFs, FailToClearDataspace)
{
  auto cap = make_unique_cap<L4Re::Dataspace>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(-L4_EACCESS, cap->clear(0, 10));
}

/**
 * Allocating memory in initial boot modules has no effect. It will
 * in particular not zero out the memory in question.
 */
TEST_F(TestMoeBootFs, FailToAllocateDataspace)
{
  auto cap = make_unique_cap<L4Re::Dataspace>();

  EXPECT_EQ(L4_EOK, ns->query("moe_bootfs_example.txt", cap.get()));
  EXPECT_EQ(0, cap->allocate(0, 10));

  // check that nothing was deleted
  L4Re::Rm::Unique_region<char *> reg;
  ASSERT_EQ(L4_EOK, env->rm()->attach(&reg, L4_PAGESIZE, L4Re::Rm::Search_addr,
                                      cap.get(), 0));
  ASSERT_EQ(0, memcmp(reg.get(), TESTFILE_CONTENT, strlen(TESTFILE_CONTENT)));

}

