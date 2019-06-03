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
#include <l4/atkins/l4_assert>
#include <l4/atkins/factory>

#include "moe_helpers.h"

/**
 * Parametrized test setup to test multiple name spaces.
 */
class MultiRegistration : public ::testing::TestWithParam<char> {};
INSTANTIATE_TEST_CASE_P(MultiNS, MultiRegistration, ::testing::Values(0, 1));

/**
 * An empty namespace can be queried with arbitrary names and lengths without
 * returning a result.
 *
 * \see L4Re::Namespace.query
 */
TEST(TestNamespace, QueryEmptyNS)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.query");

  auto ns = create_ns();
  auto lcap = make_unique_cap<void>();

  EXPECT_L4ERR(L4_ENOENT, ns->query("foo", lcap.get()))
    << "Query for nonexistent name.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("/foo", lcap.get()))
    << "Query for nonexistent name starting with '/'.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("foo/bar", lcap.get()))
    << "Recursive query in nonexistent namespace.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("\0", 1, lcap.get()))
    << "Query for null name.";
  EXPECT_L4ERR(L4_EINVAL, ns->query("", lcap.get())) << "Query for empty name.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("/", lcap.get()))
    << "Query for '/' as name.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("////", lcap.get()))
    << "Query for multiple '/' as name.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("\n", lcap.get()))
    << "Query for newline as name.";
  EXPECT_L4ERR(L4_ENOENT, ns->query("/rom\0bv", 7, lcap.get()))
    << "Query for name with middle null byte and full length.";
  EXPECT_L4ERR(L4_EINVAL, ns->query("a", 0, lcap.get()))
    << "Query for name with length specified as 0.";
  EXPECT_L4ERR(L4_EMSGTOOLONG, ns->query("a", 500, lcap.get()))
    << "Query for name with a longer specified length.";
}

/**
 * A valid capability may be registered under a given name and
 * will afterwards be returned under the same name.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterValid)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.query");

  auto ns = create_ns();
  auto ds = create_ds(0, 12345);
  auto lcap = make_unique_cap<L4Re::Dataspace>();

  ASSERT_L4ERR(L4_ENOENT, ns->query("example", lcap.get()))
    << "Name to be registered does not yet exist.";
  ASSERT_L4OK(ns->register_obj("example", L4::Ipc::make_cap_rws(ds.get())))
    << "Register dataspace to new name.";
  ASSERT_L4OK(ns->query("example", lcap.get()))
    << "Looking up the name returns a capability.";
  ASSERT_EQ(12345UL, lcap->size())
    << "The returned capability points to our newly created dataspace.";
}

/**
 * When registering a name with an invalid capability, queries on the
 * name will block. A valid capability can be registered afterwards under
 * the same name.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterInvalid)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.query");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();
  auto cap = make_unique_cap<void>();

  ASSERT_L4ERR(L4_ENOENT, ns->query("pend", cap.get()))
    << "Name to be registered does not yet exist.";
  ASSERT_L4OK(ns->register_obj("pend", L4::Cap<void>()))
    << "Register invalid capability in the namespace.";
  ASSERT_L4ERR(L4_EAGAIN,
               ns->query("pend", cap.get(), L4Re::Namespace::To_non_blocking))
    << "Server responds to query for the name to try again later.";
  ASSERT_EQ(0, ns->register_obj("pend", env->log()))
    << "Register an object with the name.";
  ASSERT_EQ(0, ns->query("pend", cap.get()))
    << "Looking up the name returns a capability.";
}

/**
 * A name that was registered with an invalid capability may be
 * deleted.
 *
 * \see L4Re::Namespace.register_obj, L4Re::Namespace.unlink
 */
TEST(TestNamespace, FreeInvalidEntry)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.unlink");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();

  ASSERT_EQ(0, ns->register_obj("inval", L4::Cap<void>()))
    << "Register invalid capability in the namespace.";
  ASSERT_L4OK(ns->unlink("inval"))
    << "Delete name pointing to invalid capability.";
}

/**
 * When registering an entry, the name must not contain the
 * separation mark '/'.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterWithSlash)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  ASSERT_L4ERR(L4_EINVAL, create_ns()->register_obj("ping/pong", env->log()))
    << "A name containing a '/' cannot be registered.";
}

/**
 * When registering an entry, the name may not be empty.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterEmpty)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  ASSERT_L4ERR(L4_EINVAL, create_ns()->register_obj("", env->log()))
    << "An empty name cannot be registered.";
}

/**
 * An entry with valid capability may only be overwritten with an
 * invalid capability, when the Overwrite flag is set.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterOverwriteInvalid)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();
  auto ds = create_ds();
  auto cap = make_unique_cap<void>();

  ASSERT_L4OK(ns->register_obj("f", ds.get()))
    << "Register dataspace in the namespace.";
  ASSERT_L4OK(ns->query("f", cap.get()))
    << "Looking up the name returns a capability.";
  ASSERT_L4ERR(L4_EEXIST, ns->register_obj("f", L4::Cap<void>()))
    << "It is not possible to register the name twice.";
  ASSERT_L4OK(ns->register_obj(
                "f", L4::Cap<void>(),
                L4Re::Namespace::Overwrite | L4Re::Namespace::Rw))
    << "Explicitly overwrite an already existing name in the namespace with "
       "the invalid capability.";
  ASSERT_L4ERR(L4_EAGAIN,
               ns->query("f", cap.get(), L4Re::Namespace::To_non_blocking))
    << "The name is not bound to an object.";
}

/**
 * An entry with valid capability may only be overwritten with a
 * valid capability, when the Overwrite flag is set.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterOverwriteValid)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();
  auto ds1 = create_ds(0, 6543);
  auto ds2 = create_ds(0, 1234);
  auto cap = make_unique_cap<L4Re::Dataspace>();

  ASSERT_L4OK(ns->register_obj("f", ds1.get()))
    << "Register dataspace in the namespace.";
  ASSERT_L4OK(ns->query("f", cap.get()))
    << "Looking up the name returns a capability.";
  EXPECT_EQ(6543UL, cap->size())
    << "The returned capability points to the first dataspace.";
  ASSERT_L4ERR(L4_EEXIST, ns->register_obj("f", ds2.get()))
    << "It is not possible to register the name twice.";
  ASSERT_L4OK(ns->register_obj(
                "f", ds2.get(),
                L4Re::Namespace::Overwrite | L4Re::Namespace::Rw))
    << "Explicitly overwrite an already existing name in the namespace with "
       "the invalid capability.";
  ASSERT_L4OK(ns->query("f", cap.get()))
    << "Looking up the name again returns a capability.";
  ASSERT_EQ(1234UL, cap->size())
    << "The name is now bound to the second dataspace.";
}

/**
 * An entry keeps a valid capability even when all other copies
 * of the capability in the system are removed.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterLooseSourceDataspace)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();
  auto cap = make_unique_cap<L4Re::Dataspace>();

  {
    auto ds = make_unique_cap<L4Re::Dataspace>();
    L4Re::chksys(env->mem_alloc()->alloc(999, ds.get(), 0),
                 "Allocate memory into dataspace.");

    ASSERT_L4OK(ns->register_obj("gone", ds.get()))
      << "Register dataspace in the namespace.";
  }

  ASSERT_L4OK(ns->query("gone", cap.get()))
    << "The capability can still be retrieved when all other references are "
       "gone.";
  ASSERT_EQ(999UL, cap->size())
    << "The capability returned points to the dataspace.";
}

/**
 * When the capability of an entry is deleted by someone else,
 * then the entry should become invalid.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterDeleteSourceDataspace)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.query");

  auto ns = create_ns();
  auto cap = make_unique_cap<L4Re::Dataspace>();

  {
    auto ds = make_unique_del_cap<L4Re::Dataspace>();
    L4Re::chksys(env->mem_alloc()->alloc(999, ds.get(), 0),
                 "Allocate memory into dataspace.");

    ASSERT_L4OK(ns->register_obj("_", ds.get()))
      << "Register dataspace in the namespace.";
  }

  ASSERT_L4ERR(L4_EAGAIN,
               ns->query("_", cap.get(), L4Re::Namespace::To_non_blocking))
    << "After the capability was deleted, the namespace entry is invalid.";
}


/**
 * When a capability is registered under two names, it must not be possible to
 * replace the capability under the first name by replacing the capability
 * behind the second.
 *
 * This test first uses the same namespace (rom) and then a separate one.
 *
 * \see L4Re::Namespace.register_obj, L4Re::Namespace.unlink
 */
TEST_P(MultiRegistration, ReRegisterSecondName)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.unlink");

  // get the cap for our test dataspace
  auto lcap = make_unique_cap<L4Re::Dataspace>();
  auto rom = env->get_cap<L4Re::Namespace>("rom");

  L4Re::Util::Unique_del_cap<L4Re::Namespace> new_ns;
  switch (GetParam())
    {
    case 0:
      new_ns = L4Re::Util::Unique_del_cap<L4Re::Namespace>(
        env->get_cap<L4Re::Namespace>("rom"));
      break;
    case 1:
      new_ns = create_ns();
      break;
    default:
      L4Re::chksys(L4_EINVAL, "Invalid test parameter.");
    }

  ASSERT_L4OK(rom->query("moe_bootfs_example.txt", lcap.get()))
    << "Looking up the test dataspace returns a capability.";
  unsigned long dssz = lcap->size();
  ASSERT_L4OK(new_ns->register_obj("new", lcap.get()))
    << "Register the returned capability with a new name.";
  // now get the new cap
  auto ds = Atkins::Factory::cap<L4Re::Dataspace>();
  ASSERT_EQ(L4_EOK, env->mem_alloc()->alloc(dssz * 2, ds.get()))
    << "Create a new dataspace and bind it to the existing capability.";

  ASSERT_L4OK(new_ns->register_obj(
                "new", ds.get(),
                L4Re::Namespace::Overwrite | L4Re::Namespace::Rw))
    << "Re-register the new name to the new dataspace.";

  auto ds2 = Atkins::Factory::cap<L4Re::Dataspace>();
  ASSERT_EQ(L4_EOK, rom->query("moe_bootfs_example.txt", ds2.get()))
    << "Look up the test dataspace again.";
  ASSERT_EQ(dssz, ds2->size()) << "The returned dataspace is the original one.";
  L4Re::chksys(new_ns->unlink("new"), "Clean up the new name.");
}

/**
 * A capability cannot be registered with more rights than the caller
 * possesses for the capability.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterPropagateRights)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();

  ASSERT_L4OK(ns->register_obj("first", ns.get(), L4Re::Namespace::Ro))
    << "Register a read-only namespace capability in the namespace.";

  auto ncap = make_unique_cap<L4Re::Namespace>();

  ASSERT_L4OK(ns->query("first", ncap.get()))
    << "Look up the registered name into a new namespace capability slot.";
  ASSERT_L4ERR(L4_EPERM, ncap->register_obj("foo", L4::Cap<void>()))
    << "New objects cannot be registered in a namespace with read-only rights.";
  ASSERT_L4OK(ns->register_obj("second", L4::Ipc::make_cap_rw(ncap.get()),
                               L4Re::Namespace::Rw))
    << "Try to increase rights by registering to a new name.";
  ASSERT_L4OK(ns->query("second", ncap.get())) << "Look up the second name.";
  ASSERT_L4ERR(L4_EPERM, ncap->register_obj("foo", L4::Cap<void>()))
    << "The returned namespace has still the original read-only rights.";
}

/**
 * Unknown flag bits for the registration call are ignored.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, RegisterBadFlags)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");

  auto ns = create_ns();
  auto ds = create_ds();

  ASSERT_L4OK(ns->register_obj("flagall", ds.get(), ~0U))
    << "Unknown registration flags are ignored when registering a valid "
       "capability.";
  ASSERT_L4OK(ns->register_obj("flagall2", L4::Cap<void>(), ~0U))
    << "Unknown registration flags are ignored when registering an invalid "
       "capability.";
}

/**
 * When a namespace entry is deleted its allocated memory is freed.
 *
 * The test registers namespace entries until moe reports to be out of memory
 * and then deletes exactly one entry. If the memory was freed correctly
 * it should be possible to register another name.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST(TestNamespace, ExhaustQuotaWithRegister)
{
  TAP_COMP_FUNC("Moe", "L4Re::Namespace.register_obj");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.unlink");

  auto cap = create_fab(3 * L4_PAGESIZE);
  auto ns = create_ns(cap.get());

  // Create new entries in the name space until we are out of memory.
  for (int i = 0;; ++i)
    {
      auto name = std::to_string(i);
      long ret = ns->register_obj(name.c_str(), cap.get());
      if (ret != L4_EOK)
        {
          ASSERT_L4ERR(L4_ENOMEM, ret) << "Registering new names eventually "
                                          "exhausts the available memory.";
          ASSERT_GT(i, 0) << "At least one name entry was created.";
          name = std::to_string(i - 1);
          ASSERT_EQ(0, ns->unlink(name.c_str())) << "Delete the last entry.";
          break;
        }
    }

  EXPECT_EQ(0, ns->register_obj("x", cap.get()))
    << "Space for registering a new name entry has been made available.";
}

/**
 * When a namespace is deleted its allocated memory is freed.
 *
 * The test allocates namespaces until moe reports to be out of memory
 * and then deletes exactly one namespace. If the memory was freed correctly
 * it should be possible to allocate a new namespace.
 *
 * \see L4::Factory.create
 */
TEST(TestNamespace, ExhaustQuotaWithCreate)
{
  TAP_COMP_FUNC("Moe", "L4::Factory.create");
  TAP_COMP_FUNC2("Moe", "L4Re::Namespace.register_obj");

  auto cap = create_fab(3 * L4_PAGESIZE);

  // Create namespaces without deleting them until we are out of memory
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
              ASSERT_L4ERR(L4_ENOMEM, ret)
                << "Registering a new name exhausts the available memory.";
              break;
            }
          nslist.push_back(ns);
        }
      else
        {
          ASSERT_L4ERR(L4_ENOMEM, ret)
            << "Registering new namespaces eventually exhausts the available "
               "memory.";
          break;
        }
    }

  ASSERT_FALSE(nslist.empty())
    << "At least one new namespace has been created.";
  // free the previously allocated namespace
  nslist.pop_back();

  // after freeing, we should be able to allocate again
  auto ns = make_unique_del_cap<L4Re::Namespace>();
  ASSERT_EQ(0, l4_error(cap->create(ns.get(), L4Re::Namespace::Protocol)))
    << "Space for creating a new namespace has been made available.";
  ASSERT_EQ(0, ns->register_obj("x", cap.get()))
    << "Register a name in the new namespace.";
}
