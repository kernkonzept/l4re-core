/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/atkins/tap/main>

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/re/namespace>

static L4Re::Env const *env = L4Re::Env::env();
static L4::Cap<L4Re::Namespace> rom_ns = env->get_cap<L4Re::Namespace>("rom");

TEST(CapAlloc, AutoCap)
{
  L4::Cap<L4Re::Namespace> cap;

    {
      auto autocap = L4Re::Util::make_auto_cap<L4Re::Namespace>();
      ASSERT_TRUE(autocap.is_valid());
      // remember the slot for later
      cap = autocap.get();

      // initially the cap is invalid
      ASSERT_EQ(0, env->task()->cap_valid(cap).label());
      // map a legal cap into the autocap slot
      ASSERT_EQ(0, rom_ns->query("l4re", cap));
      ASSERT_NE(0, env->task()->cap_valid(cap).label());
    }
  // now that we are out of scope, the slot should no longer contain a cap
  ASSERT_EQ(0, env->task()->cap_valid(cap).label());
}

TEST(CapAlloc, AutoDelCap)
{
  auto testcap = L4Re::Util::make_auto_cap<L4Re::Dataspace>();

    {
      auto autocap = L4Re::Util::make_auto_del_cap<L4Re::Dataspace>();
      ASSERT_TRUE(autocap.is_valid());

      // create a capability and register the cap back into the rom namespace
      ASSERT_EQ(0, env->mem_alloc()->alloc(1024, autocap.get()));
      ASSERT_EQ(0, rom_ns->register_obj("XXX", autocap.get()));
      // it should be possible to query out new object
      ASSERT_EQ(0, rom_ns->query("XXX", testcap.get()));
    }
  // At the end of the scope, the cap should have been cleaned up
  // and therefore must not be available in the rom namespace anymore
  ASSERT_EQ(-L4_EAGAIN, rom_ns->query("XXX", testcap.get(),
                                      L4Re::Namespace::To_non_blocking));
}

TEST(CapAlloc, RefCap)
{
  L4::Cap<L4Re::Namespace> cap;
    {
      L4Re::Util::Ref_cap<L4Re::Namespace>::Cap global_cap;

        {
          auto refcap = L4Re::Util::make_ref_cap<L4Re::Namespace>();
          ASSERT_TRUE(refcap.is_valid());
          // remember the slot for later
          cap = refcap.get();
          // increase ref count
          global_cap = refcap;

          // initially the cap is invalid
          ASSERT_EQ(0, env->task()->cap_valid(cap).label());
          // map a legal cap into the refcap slot
          ASSERT_EQ(0, rom_ns->query("l4re", cap));
          ASSERT_NE(0, env->task()->cap_valid(cap).label());
        }
      // with refcap gone, the ref counter goes down to one
      // cap is still valid
      ASSERT_NE(0, env->task()->cap_valid(cap).label());
    }

  // now that both refs are out of scope,
  // the slot should no longer contain a cap
  ASSERT_EQ(0, env->task()->cap_valid(cap).label());
}

TEST(CapAlloc, RefDelCap)
{
  auto testcap = L4Re::Util::make_auto_cap<L4Re::Dataspace>();
    {
      L4Re::Util::Ref_del_cap<L4Re::Dataspace>::Cap global_ds_cap;

        {
          auto refcap = L4Re::Util::make_ref_del_cap<L4Re::Dataspace>();
          ASSERT_TRUE(refcap.is_valid());
          // increase ref count
          global_ds_cap = refcap;

          // create a capability and register the cap back into the rom namespace
          ASSERT_EQ(0, env->mem_alloc()->alloc(1024, refcap.get()));
          ASSERT_EQ(0, rom_ns->register_obj("XXX", refcap.get()));
          // it should be possible to query out new object
          ASSERT_EQ(0, rom_ns->query("XXX", testcap.get()));
        }
      // with refcap gone, the ref counter goes down to one
      // the object should still be queryable in the namespace
      ASSERT_EQ(0, rom_ns->query("XXX", testcap.get()));
    }
  // At the end of the scope, the cap should have been cleaned up
  // and therefore must not be available in the rom namespace anymore
  ASSERT_EQ(-L4_EAGAIN, rom_ns->query("XXX", testcap.get(),
                                     L4Re::Namespace::To_non_blocking));

}
