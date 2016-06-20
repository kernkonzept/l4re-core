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

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/namespace>
#include <l4/re/error_helper>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"

class TestNamespace : public ::testing::Test {};

TEST_F(TestNamespace, QueryEmptyNS)
{
  auto ns = create_ns();
  auto lcap = make_auto_cap<void>();

  EXPECT_EQ(-L4_ENOENT, ns->query("foo", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("/foo", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("foo/bar", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("\0", 1, lcap.get()));
  EXPECT_EQ(-L4_EINVAL, ns->query("", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("/", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("////", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("\n", lcap.get()));
  EXPECT_EQ(-L4_ENOENT, ns->query("ab\0bv", 5, lcap.get()));
  EXPECT_EQ(-L4_EINVAL, ns->query("a", 0, lcap.get()));
  EXPECT_EQ(-L4_EMSGTOOLONG, ns->query("a", 500, lcap.get()));
}

TEST_F(TestNamespace, RegisterValid)
{
  auto ns = create_ns();
  auto ds = create_ds(0, 12345);
  auto lcap = make_auto_cap<L4Re::Dataspace>();

  ASSERT_EQ(-L4_ENOENT, ns->query("example", lcap.get()));
  ASSERT_EQ(L4_EOK, ns->register_obj("example",
                                     L4::Ipc::make_cap_rws(ds.get())));
  ASSERT_EQ(L4_EOK, ns->query("example", lcap.get()));
  // check that this is our ds
  ASSERT_EQ(12345UL, lcap->size());
}

TEST_F(TestNamespace, RegisterInvalid)
{
  auto ns = create_ns();
  auto cap = make_auto_cap<void>();

  ASSERT_EQ(-L4_ENOENT, ns->query("pend", cap.get()));
  // Register just the name.
  ASSERT_EQ(L4_EOK, ns->register_obj("pend", L4::Cap<void>()));
  // Server should tell us to try again later.
  ASSERT_EQ(-L4_EAGAIN, ns->query("pend", cap.get(),
                                  L4Re::Namespace::To_non_blocking));
  // Now register the correct one.
  ASSERT_EQ(0, ns->register_obj("pend", env->log()));
  // And we should get back the capability.
  ASSERT_EQ(0, ns->query("pend", cap.get()));
}

TEST_F(TestNamespace, FreeInvalidEntry)
{
  auto ns = create_ns();

  ASSERT_EQ(0, ns->register_obj("inval", L4::Cap<void>()));
  EXPECT_EQ(L4_EOK, ns->unlink("inval"));
}

TEST_F(TestNamespace, RegisterWithSlash)
{
  EXPECT_EQ(-L4_EINVAL, create_ns()->register_obj("ping/pong", env->log()));
}

TEST_F(TestNamespace, RegisterEmpty)
{
  ASSERT_EQ(-L4_EINVAL, create_ns()->register_obj("", env->log()));
}

TEST_F(TestNamespace, RegisterOverwriteInvalid)
{
  auto ns = create_ns();
  auto ds = create_ds();
  auto cap = make_auto_cap<void>();

  ASSERT_EQ(L4_EOK, ns->register_obj("f", ds.get()));
  ASSERT_EQ(L4_EOK, ns->query("f", cap.get()));
  ASSERT_EQ(-L4_EEXIST, ns->register_obj("f", L4::Cap<void>()));
  ASSERT_EQ(L4_EOK, ns->register_obj("f", L4::Cap<void>(),
                                     L4Re::Namespace::Overwrite |
                                     L4Re::Namespace::Rw));
  ASSERT_EQ(-L4_EAGAIN, ns->query("f", cap.get(),
                                  L4Re::Namespace::To_non_blocking));
}

TEST_F(TestNamespace, RegisterOverwriteValid)
{
  auto ns = create_ns();
  auto ds1 = create_ds(0, 6543);
  auto ds2 = create_ds(0, 1234);
  auto cap = make_auto_cap<L4Re::Dataspace>();

  ASSERT_EQ(L4_EOK, ns->register_obj("f", ds1.get()));
  ASSERT_EQ(L4_EOK, ns->query("f", cap.get()));
  EXPECT_EQ(6543UL, cap->size());
  ASSERT_EQ(-L4_EEXIST, ns->register_obj("f", ds2.get()));
  ASSERT_EQ(L4_EOK, ns->register_obj("f", ds2.get(),
                                     L4Re::Namespace::Overwrite |
                                     L4Re::Namespace::Rw));
  ASSERT_EQ(L4_EOK, ns->query("f", cap.get()));
  EXPECT_EQ(1234UL, cap->size());
}

TEST_F(TestNamespace, RegisterLooseSourceDataspace)
{
  auto ns = create_ns();
  auto cap = make_auto_cap<L4Re::Dataspace>();

    {
      auto ds = make_auto_cap<L4Re::Dataspace>();
      L4Re::chksys(env->mem_alloc()->alloc(999, ds.get(), 0));

      ASSERT_EQ(L4_EOK, ns->register_obj("gone", ds.get()));
    }

  // if all references to the original cap are lost, the original
  // cap should remain accessible
  ASSERT_EQ(L4_EOK, ns->query("gone", cap.get()));
  EXPECT_EQ(999UL, cap->size());
}

TEST_F(TestNamespace, RegisterDeleteSourceDataspace)
{
  auto ns = create_ns();
  auto cap = make_auto_cap<L4Re::Dataspace>();

    {
      auto ds = make_auto_del_cap<L4Re::Dataspace>();
      L4Re::chksys(env->mem_alloc()->alloc(999, ds.get(), 0));

      ASSERT_EQ(L4_EOK, ns->register_obj("_", ds.get()));
    }

  // when the original cap is deleted, the entry should be set to invalid
  ASSERT_EQ(-L4_EAGAIN, ns->query("_", cap.get(),
                                  L4Re::Namespace::To_non_blocking));
}


TEST_F(TestNamespace, RegisterDeleteRomDataspace)
{
  auto ns = create_ns();
  auto lcap = make_auto_cap<L4Re::Dataspace>();

  // get the cap for our test dataspace
  auto rom = env->get_cap<L4Re::Namespace>("rom");
  ASSERT_EQ(L4_EOK, rom->query("moe_bootfs_example.txt", lcap.get()));
  unsigned long dssz = lcap->size();
  // register it with the new namespace
  ASSERT_EQ(L4_EOK, ns->register_obj("new", lcap.get()));
  // now get the new cap
  auto ds = L4Re::chkcap(L4Re::Util::make_auto_cap<L4Re::Dataspace>());
  ASSERT_EQ(L4_EOK, ns->query("new", ds.get()));
  // and delete it again
  ASSERT_EQ(L4_EOK, ns->unlink("new"));
  // If the delete screwed up and the capability was freed, a new alloc
  // should be able to reclaim the capability without the namespace noticing.
  // Create a new dataspace to try reclaiming the capability.
  ASSERT_EQ(L4_EOK, env->mem_alloc()->alloc(dssz * 2, ds.get()));

  // Now ask again for the test namespace.
  auto ds2 = L4Re::chkcap(L4Re::Util::make_auto_cap<L4Re::Dataspace>());
  ASSERT_EQ(L4_EOK, rom->query("moe_bootfs_example.txt", ds2.get()));
  // The size of the underlying dataspace should not have changed.
  ASSERT_GT(ds2.validate(L4_BASE_TASK_CAP).label(), 0);
  ASSERT_EQ(dssz, ds2->size());
  // Sanity check that the unlink has worked.
  ASSERT_EQ(-L4_ENOENT, ns->unlink("new"));
}

TEST_F(TestNamespace, RegisterPropagateRights)
{
  auto ns = create_ns();

  // Register capability read-only.
  ASSERT_EQ(L4_EOK, ns->register_obj("first", ns.get(), L4Re::Namespace::Ro));

  auto ncap = make_auto_cap<L4Re::Namespace>();

  ASSERT_EQ(L4_EOK, ns->query("first", ncap.get()));
  // We should not be allowed to register on the returned capability.
  ASSERT_EQ(-L4_EPERM, ncap->register_obj("foo", L4::Cap<void>()));
  // Register the cap under a new name with full rights.
  ASSERT_EQ(L4_EOK, ns->register_obj("second", L4::Ipc::make_cap_rw(ncap.get()),
                                     L4Re::Namespace::Rw));
  // We still shouldn't be able to register on that second cap.
  ASSERT_EQ(L4_EOK, ns->query("second", ncap.get()));
  ASSERT_EQ(-L4_EPERM, ncap->register_obj("foo", L4::Cap<void>()));
}

TEST_F(TestNamespace, RegisterBadFlags)
{
  auto ns = create_ns();
  auto ds = create_ds();

  ASSERT_EQ(L4_EOK, ns->register_obj("flagall", ds.get(), ~0U));
  ASSERT_EQ(L4_EOK, ns->register_obj("flagall2", L4::Cap<void>(), ~0U));
}

TEST_F(TestNamespace, ExhaustQuotaWithRegister)
{
  auto cap = create_fab(3 * L4_PAGESIZE);
  auto ns = create_ns(cap.get());

  // Create new entries in the name space until we are out of memory.
  for (int i = 0;; ++i)
    {
      auto name = std::to_string(i);
      long ret = ns->register_obj(name.c_str(), cap.get());
      if (ret != L4_EOK)
        {
          ASSERT_EQ(-L4_ENOMEM, ret);
          ASSERT_GT(i, 0);
          // now delete the last created entry.
          name = std::to_string(i - 1);
          ASSERT_EQ(0, ns->unlink(name.c_str()));
          break;
        }
    }

  // after freeing, we should be able to register again
  EXPECT_EQ(0, ns->register_obj("x", cap.get()));
}


TEST_F(TestNamespace, ExhaustQuotaWithCreate)
{
  auto cap = create_fab(3 * L4_PAGESIZE);

  // Create dataspaces without deleting them until we are out of memory
  std::vector<L4Re::Util::Ref_cap<L4Re::Namespace>::Cap> nslist;

  for (;;)
    {
      auto ns = make_ref_cap<L4Re::Namespace>();

      long ret = l4_error(cap->create(ns.get(), L4Re::Namespace::Protocol));
      if (ret == L4_EOK)
        {
          ret = ns->register_obj("x", cap.get());
          if (ret != L4_EOK)
            {
              ASSERT_EQ(-L4_ENOMEM, ret);
              break;
            }
          nslist.push_back(ns);
        }
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
  auto ns = make_auto_del_cap<L4Re::Namespace>();
  ASSERT_EQ(0, l4_error(cap->create(ns.get(), L4Re::Namespace::Protocol)));
  ASSERT_EQ(0, ns->register_obj("x", cap.get()));
}
