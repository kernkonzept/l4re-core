/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Timo Nicolai <timo.nicolai@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * This file tests the functionality of the Loader:start() function available
 * in Ned-scripts.
 *
 * It depends on the Lua script 'start.cfg' in the same directory.
 */
#include <gtest/gtest.h>

#include <l4/atkins/factory>
#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>
#include <l4/atkins/tap/tap>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/debug>
#include <l4/sys/ipc_gate>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

using L4Re::chkcap;
using L4Re::chksys;

/**
 * Check if a namespace capability has write rights.
 *
 * It is tested whether it is possible to register other capabilities in the
 * namespace through this capability.
 */
static bool
has_permission_w(L4::Cap<L4Re::Namespace> cap)
{
  // Register an invalid dummy capability.
  L4::Ipc::Cap<void> dummy_cap;
  long tmp = cap->register_obj("dummy", dummy_cap);

  if (tmp == -L4_EPERM)
    return false;

  chksys(tmp, "Register a dummy capability in namespace.");
  return true;
}

/**
 * Check if a factory capability has 'special' rights.
 *
 * It is tested whether it is possible to create a namespace using the factory
 * capability.
 */
static bool
has_permission_s(L4::Cap<L4::Factory> cap)
{
  auto dummy_cap =
    Atkins::Factory::cap<L4Re::Namespace>("Allocate dummy capability");

  long err = l4_error(cap->create(dummy_cap.get()));

  if (err == -L4_EPERM)
    return false;

  chksys(err, "Create a dummy namespace.");
  return true;
}

/**
 * Check if a capability has delete rights.
 *
 * First, a copy of the capability is created by mapping it into this task
 * again, deleting the original capability and then checking whether the copy
 * is still present after that (which it should not be if the original
 * capability had delete rights).
 */
static bool
has_permission_d(L4::Cap<void> cap)
{
  L4::Cap<L4::Task> this_task_cap =
    chkcap(L4Re::Env::env()->task(), "Obtain task capability.");

  auto cap_copy = Atkins::Factory::cap<void>("Allocate capability");

  chksys(this_task_cap->map(this_task_cap, cap.fpage(), cap_copy.snd_base()),
         "Map capability into this task again.");

  auto map_err = l4_error(this_task_cap->cap_valid(cap_copy.get()));
  if (map_err <= 0)
    throw L4::Runtime_error(-L4_ENOSYS,
                            "Mapped capability refers to an object.");

  chksys(this_task_cap->delete_obj(cap), "Delete original capability.");

  auto del_tag = this_task_cap->cap_valid(cap_copy.get());
  chksys(del_tag,
         "Obtain validity of capability copy after deletion of original");

  return del_tag.label() != 1;
}

/**
 * Namespaces can be passed to this task via Loader:start().
 *
 * Namespace definitions given in entries of the 'caps' table contained in the
 * first argument of Loader:start() should be converted into valid namespace
 * capabilities accessible to this task.  All entries of 'caps' whose values
 * are tables mapping strings to capabilities/strings should be interpreted as
 * namespace definitions. For one such example it is verified that a
 * corresponding valid namespace capability is available to this task and that
 * the correct capabilities are returned when the namespace is queried for
 * their corresponding keys. It is also verified that the right error
 * identifier is returned if keys whose values were given as placeholder
 * strings are queried.
 */
TEST(ReceivedCapabilities, ReceivedNamespace)
{
  L4::Cap<L4Re::Namespace> ns_cap =
    L4Re::Env::env()->get_cap<L4Re::Namespace>("test_namespace");

  ASSERT_L4CAP_PRESENT(ns_cap);

  char const *const expected_entries[][2] = {{"name1", "dummy_cap1"},
                                             {"name2", "dummy_cap2"}};

  auto query_cap = Atkins::Factory::cap<void>("Allocate capability");

  for (auto entry : expected_entries)
    {
      char const *ns_name = entry[0];
      char const *cap_name = entry[1];

      ASSERT_L4OK(ns_cap->query(ns_name, query_cap.get()))
        << "Query for capability registered under the name '" << ns_name
        << "'.";

      L4::Cap<void> reference_cap =
        chkcap(L4Re::Env::env()->get_cap<void>(cap_name),
               "Get reference capability for query from environment table.");

      EXPECT_L4CAP_OBJ_EQ(reference_cap, query_cap.get())
        << "Compare capability returned by query with expected capability.";
    }

  EXPECT_EQ(-L4_EAGAIN, ns_cap->query("name_ph", query_cap.get(),
                                      L4Re::Namespace::To_non_blocking))
    << "The namespace contains a placeholder entry for key 'name_ph'.";
}

/**
 * This task's 'rom' capability is read-only.
 *
 * Each task should implicitly be given access to a capability to the 'rom'
 * namespace via its environment table. This capability should have read-only
 * rights.
 */
TEST(ReceivedCapabilities, ReceivedReadonlyRom)
{
  L4::Cap<L4Re::Namespace> rom =
    chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("rom"));

  ASSERT_L4CAP_PRESENT(rom);

  EXPECT_FALSE(has_permission_w(rom)) << "Rom capability is read-only.";
}

/**
 * Capabilities passed to this task have correct write permissions.
 *
 * Capabilities made available to this task via entries of the 'caps' table in
 * the first argument of the Loader:start() function that has spawned this task
 * must have write rights if and only if their 'mode' was set to include them
 * in the corresponding Ned-script.
 * Two namespace capabilities, one with it's mode set to include write rights
 * and another with it's mode set to be read-only are made available to this
 * task and it is verified via has_permission_w() that this task has the
 * corresponding correct access rights to these capabilities.
 */
TEST(ReceivedCapabilities, CorrectWritePermissions)
{
  L4::Cap<L4Re::Namespace> test_w =
    chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("test_w"));
  L4::Cap<L4Re::Namespace> test_no_w =
    chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("test_no_w"));

  ASSERT_L4CAP_PRESENT(test_w);
  EXPECT_TRUE(has_permission_w(test_w)) << "Namespace has write permissions.";

  ASSERT_L4CAP_PRESENT(test_no_w);
  EXPECT_FALSE(has_permission_w(test_no_w))
    << "Namespace does not have write permissions.";
}

/**
 * Capabilities passed to this task have correct 'special' permissions.
 *
 * Capabilities made available to this task via entries of the 'caps' table in
 * the first argument of the Loader:start() function that has spawned this task
 * must have 'special' rights if and only if their 'mode' was set to include
 * them in the corresponding Ned-script.
 * Two factory capabilities, one with it's mode set to include special rights
 * and another with it's mode set to be read-only are made available to this
 * task and it is verified via has_permission_s() that this task has the
 * corresponding correct access rights to these capabilities.
 */
TEST(ReceivedCapabilities, CorrectSpecialPermissions)
{
  L4::Cap<L4::Factory> test_s =
    chkcap(L4Re::Env::env()->get_cap<L4::Factory>("test_s"));
  L4::Cap<L4::Factory> test_no_s =
    chkcap(L4Re::Env::env()->get_cap<L4::Factory>("test_no_s"));

  ASSERT_L4CAP_PRESENT(test_s);
  EXPECT_TRUE(has_permission_s(test_s))
    << "Factory has 'special' permissions.";

  ASSERT_L4CAP_PRESENT(test_no_s);
  EXPECT_FALSE(has_permission_s(test_no_s))
    << "Factory does not have 'special' permissions.";
}

/**
 * Capabilities passed to this task have correct delete permissions.
 *
 * Capabilities made available to this task via entries of the 'caps' table in
 * the first argument of the Loader:start() function that has spawned this task
 * must have delete rights if and only if their 'mode' was set to include them
 * in the corresponding Ned-script.
 * Two capabilities of arbitrary type, one with it's mode set to include
 * delete rights and another with it's mode set to be read-only are made
 * available to this task and it is verified via has_permission_d() that this
 * task has the corresponding correct access rights to these capabilities.
 */
TEST(ReceivedCapabilities, CorrectDeletePermissions)
{
  L4::Cap<void> test_d = chkcap(L4Re::Env::env()->get_cap<void>("test_d"));
  L4::Cap<void> test_no_d =
    chkcap(L4Re::Env::env()->get_cap<void>("test_no_d"));

  ASSERT_L4CAP_PRESENT(test_d);
  EXPECT_TRUE(has_permission_d(test_d)) << "Capability has delete permissions.";

  ASSERT_L4CAP_PRESENT(test_no_d);
  EXPECT_FALSE(has_permission_d(test_no_d))
    << "Capability does not have delete permissions.";
}
