/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *            Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test for Unique_cap, Unique_del_cap, Shared_cap, Shared_del_cap.
 */
#include <l4/sys/thread>

#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/atkins/factory>

#include <l4/re/unique_cap>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/util/shared_cap>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/error_helper>

static L4Re::Env const *env = L4Re::Env::env();
static L4::Cap<L4Re::Namespace> rom_ns = env->get_cap<L4Re::Namespace>("rom");

/**
 * An Unique_cap allocates a cap slot on creation and unmaps the capability when
 * it goes out of scope.
 *
 * \see L4Re::Util::Unique_cap
 */
TEST(CapAlloc, UniqueCap)
{
  L4::Cap<L4Re::Namespace> cap;

    {
      auto uniquecap =
        Atkins::Factory::cap<L4Re::Namespace>("Allocate a namespace cap.");
      // remember the slot for later
      cap = uniquecap.get();

      ASSERT_L4CAP_NOT_PRESENT(cap)
        << "Initially the capability does not point to a kernel object.";
      ASSERT_EQ(0, rom_ns->query("l4re", cap))
        << "Map a legal capability into the uniquecap slot.";
      ASSERT_L4CAP_PRESENT(cap)
        << "After mapping a capability points to a kernel object.";
    }
    ASSERT_L4CAP_NOT_PRESENT(cap) << "Now that we are out of scope, the slot "
                                     "should not point to a kernel object.";
}

/**
 * An Unique_del_cap deletes the referenced object when it goes out of scope.
 *
 * \see L4Re::Util::Unique_del_cap
 */
TEST(CapAlloc, UniqueDelCap)
{
  auto testcap = L4Re::Util::make_unique_cap<L4Re::Dataspace>();

    {
      auto uniquecap =
        Atkins::Factory::del_cap<L4Re::Dataspace>("Allocate a dataspace cap.");

      ASSERT_EQ(0, env->mem_alloc()->alloc(1024, uniquecap.get()))
        << "Allocate memory in the dataspace.";
      ASSERT_EQ(0, rom_ns->register_obj("XXX", uniquecap.get()))
        << "Register the dataspace capability back into the 'rom' namespace.";
      ASSERT_EQ(0, rom_ns->query("XXX", testcap.get()))
        << "Validate the presence of the newly registered object.";
    }

  ASSERT_EQ(-L4_EAGAIN, rom_ns->query("XXX", testcap.get(),
                                      L4Re::Namespace::To_non_blocking))
    << "At the end of the scope, the cap is cleaned up and is therefore not "
       "available in the 'rom' namespace anymore.";
}

/**
 * A Shared_cap unmaps its managed capability when the reference count reaches
 * zero due to itself going out of scope.
 *
 * \see L4Re::Util::Shared_cap
 */
TEST(CapAlloc, SharedCap)
{
  L4::Cap<L4Re::Namespace> cap;
    {
      L4Re::Util::Shared_cap<L4Re::Namespace> global_cap;

        {
          auto sharedcap =
            Atkins::Factory::shared_cap<L4Re::Namespace>(
              "Allocate a namespace cap.");
          // remember the slot for later
          cap = sharedcap.get();
          // increase ref count
          global_cap = sharedcap;

          ASSERT_L4CAP_NOT_PRESENT(cap)
            << "Initially the capability does not point to a kernel object.";
          ASSERT_EQ(0, rom_ns->query("l4re", cap))
            << "Map a legal capability into the sharedcap slot.";
          ASSERT_L4CAP_PRESENT(cap)
            << "After mapping a capability points to a kernel object.";
        }

      ASSERT_L4CAP_PRESENT(cap)
        << "After the shared cap goes out of scope, the ref counter goes "
           "down to one. The cap is still valid and points to a kernel "
           "object.";
    }

  ASSERT_L4CAP_NOT_PRESENT(cap) << "After all shared caps go out of scope, "
                                   "the capability slot is no longer valid.";
}

/**
 * A Shared_cap deletes its managed capability when the reference count reaches
 * zero due to itself going out of scope.
 *
 * \see L4Re::Util::Shared_del_cap
 */
TEST(CapAlloc, SharedDelCap)
{
  auto testcap =
    Atkins::Factory::cap<L4Re::Dataspace>("Allocate dataspace cap.");

    {
      L4Re::Util::Shared_del_cap<L4Re::Dataspace> global_ds_cap;

        {
          auto sharedcap = Atkins::Factory::shared_del_cap<L4Re::Dataspace>(
            "Allocate dataspace cap with automatic deletion.");
          // increase ref count
          global_ds_cap = sharedcap;

          // create a capability and register the cap back into the rom namespace
          ASSERT_EQ(0, env->mem_alloc()->alloc(1024, sharedcap.get()))
            << "Allocate memory in the dataspace.";
          ASSERT_EQ(0, rom_ns->register_obj("XXX", sharedcap.get()))
            << "Register the capability in the 'rom' namespace.";
          ASSERT_EQ(0, rom_ns->query("XXX", testcap.get()))
            << "Query the newly registered capability.";
        }

      ASSERT_EQ(0, rom_ns->query("XXX", testcap.get()))
        << "After the scope of the shared cap ends the reference counter "
           "decrease to one and the object is still present in the 'rom' "
           "namespace.";
    }

  ASSERT_EQ(-L4_EAGAIN, rom_ns->query("XXX", testcap.get(),
                                      L4Re::Namespace::To_non_blocking))
    << "After the scope of the last shared deletion capability ends the "
       "capability is cleaned up and must not be available in the 'rom' "
       "namespace anymore.";
}

/**
 * The move assignment of the unique_cap must delete the previous content of
 * target and invalidate the source.
 *
 * \see L4Re::Util::Unique_cap
 */
TEST(UniqueCap, MoveAssignment)
{
  auto uniquecap =
    Atkins::Factory::kobj<L4::Thread>("Create a thread object.");
  auto uniquecap_content = uniquecap.get();

  ASSERT_EQ(uniquecap.get().cap(), uniquecap_content.cap())
    << "The capability index of both cap objects is equal.";
  ASSERT_L4CAP_PRESENT(uniquecap.get())
    << "Kernel object of the capability stored in the unique_cap is present.";
  ASSERT_L4CAP_PRESENT(uniquecap_content)
    << "Kernel object of the capability content of the unique_cap is present.";

  auto newuniquecap =
    Atkins::Factory::cap<L4::Thread>("Allocate a thread capability.");
  uniquecap = std::move(newuniquecap);

  ASSERT_FALSE(newuniquecap.is_valid())
    << "Capability was moved, the source unique_cap container must be "
       "invalidated.";
  ASSERT_NE(uniquecap.cap(), uniquecap_content.cap())
    << "Capability index of the new capability in uniquecap is different than "
       "the index of the old content.";
  ASSERT_L4CAP_NOT_PRESENT(uniquecap_content)
    << "Kernel object of the capability previously stored in the unique_cap is"
       " not present.";
}
