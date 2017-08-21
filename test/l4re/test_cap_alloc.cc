/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Tests for L4Re::Util::Counting_cap_alloc.
 */
#include <l4/atkins/tap/main>

#include <l4/sys/types.h>
#include <l4/re/util/counting_cap_alloc>
#include <l4/re/util/bitmap_cap_alloc>

#include <cstring>

struct CountingCapAlloc
: L4Re::Util::Counting_cap_alloc<>,
  ::testing::Test
{
  typedef L4Re::Util::Counting_cap_alloc<>::Counter_storage<128> Storage;

  CountingCapAlloc() { memset(&cap_storage._buf, 0, sizeof(Storage)); }

  Storage cap_storage;
};

/**
 * Allocating from an uninitialized capability allocator returns an invalid
 * capability.
 *
 * \see L4Re::Util::Counting_cap_alloc
 */
TEST_F(CountingCapAlloc, UnitialisedStorage)
{
  ASSERT_FALSE(alloc<void>().is_valid());
}

/**
 * Allocating from an initialized capability allocator returns a valid
 * capability. Free returns the capability to the allocator.
 *
 * \see L4Re::Util::Counting_cap_alloc.setup,
 *      L4Re::Util::Counting_cap_alloc.alloc
 *      L4Re::Util::Counting_cap_alloc.free
 */
TEST_F(CountingCapAlloc, AllocFreeCap)
{
  setup(&cap_storage._buf, 10, 1000);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());
  ASSERT_TRUE(free(cap));
}

/**
 * Allocating from an initialized capability allocator returns a valid
 * capability. Free returns the capability to the allocator.
 *
 * \see L4Re::Util::Counting_cap_alloc.alloc
 *      L4Re::Util::Counting_cap_alloc.free
 */
TEST_F(CountingCapAlloc, AllocFreeCapWithTask)
{
  setup(&cap_storage, 10, 1000);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());
  ASSERT_TRUE(free(cap, L4Re::This_task));
}

/**
 * take and release increase and decrease the reference count of a capability.
 * When the reference counter hits zero during a release the capability slot is
 * freed.
 *
 * \see L4Re::Util::Counting_cap_alloc.take
 *      L4Re::Util::Counting_cap_alloc.release
 */
TEST_F(CountingCapAlloc, TakeReleaseCap)
{
  setup(&cap_storage, 10, 1000);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());

  take(cap);

  ASSERT_FALSE(release(cap));
  ASSERT_TRUE(release(cap));

  // now the cap slot should be available again
  auto cap2 = alloc<L4::Kobject>();

  ASSERT_EQ(cap.cap() >> L4_CAP_SHIFT, cap2.cap() >> L4_CAP_SHIFT);
}

/**
 * The bias passed to setup a capability allocator describes the first cap
 * slot to allocate.
 *
 * \see L4Re::Util::Counting_cap_alloc.setup
 */
TEST_F(CountingCapAlloc, SetupWithBias)
{
  setup(&cap_storage, 10, 333);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());

  ASSERT_GE(cap.cap() >> L4_CAP_SHIFT, 333U);
}

/**
 * An capability allocator allocates only up to its capacity limit.
 *
 * \see L4Re::Util::Counting_cap_alloc.setup,
 *      L4Re::Util::Counting_cap_alloc.alloc
 */
TEST_F(CountingCapAlloc, RespectCapacity)
{
  setup(&cap_storage, 3, 1000);

  ASSERT_TRUE(alloc<void>().is_valid());
  ASSERT_TRUE(alloc<void>().is_valid());
  ASSERT_TRUE(alloc<void>().is_valid());
  ASSERT_FALSE(alloc<void>().is_valid());
}
