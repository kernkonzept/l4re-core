/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
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

TEST_F(CountingCapAlloc, UnitialisedStorage)
{
  ASSERT_FALSE(alloc<void>().is_valid());
}

TEST_F(CountingCapAlloc, AllocFreeCap)
{
  setup(&cap_storage._buf, 10, 1000);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());
  ASSERT_TRUE(free(cap));
}

TEST_F(CountingCapAlloc, AllocFreeCapWithTask)
{
  setup(&cap_storage, 10, 1000);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());
  ASSERT_TRUE(free(cap, L4Re::This_task));
}

TEST_F(CountingCapAlloc, TakeReleaseCap)
{
  setup(&cap_storage, 10, 1000);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());

  take(cap);

  ASSERT_FALSE(release(cap));
  ASSERT_TRUE(release(cap));

  // now the cap should be available again
  auto cap2 = alloc<L4::Kobject>();

  ASSERT_EQ(cap.cap() >> L4_CAP_SHIFT, cap2.cap() >> L4_CAP_SHIFT);
}

TEST_F(CountingCapAlloc, SetupWithBias)
{
  setup(&cap_storage, 10, 333);

  auto cap = alloc<L4::Kobject>();

  ASSERT_TRUE(cap.is_valid());

  ASSERT_GE(cap.cap() >> L4_CAP_SHIFT, 333U);
}

TEST_F(CountingCapAlloc, RespectCapacity)
{
  setup(&cap_storage, 3, 1000);

  ASSERT_TRUE(alloc<void>().is_valid());
  ASSERT_TRUE(alloc<void>().is_valid());
  ASSERT_TRUE(alloc<void>().is_valid());
  ASSERT_FALSE(alloc<void>().is_valid());
}
