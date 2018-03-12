/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test parameter alignment in RPC.
 */

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, in64_in32_in8, (l4_uint64_t, l4_uint32_t, l4_uint8_t));
  L4_INLINE_RPC(long, in8_in32_in64, (l4_uint8_t, l4_uint32_t, l4_uint64_t));

  L4_INLINE_RPC(long, in8_in32_ptr, (l4_uint8_t const *, l4_uint32_t const *));

  L4_INLINE_RPC(long, o64_o32_o8, (l4_uint64_t *, l4_uint32_t *, l4_uint8_t *));
  L4_INLINE_RPC(long, o8_o32_o64, (l4_uint8_t *, l4_uint32_t *, l4_uint64_t *));


  typedef L4::Typeid::Rpcs<in64_in32_in8_t, in8_in32_in64_t,
                           in8_in32_ptr_t, o64_o32_o8_t, o8_o32_o64_t
                          > Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_in64_in32_in8(Test_iface::Rights, l4_uint64_t a,
                        l4_uint32_t b, l4_uint8_t c)
  {
    p64 = a;
    p32 = b;
    p8 = c;

    return 0;
  }

  long op_in8_in32_in64(Test_iface::Rights, l4_uint8_t a,
                        l4_uint32_t b, l4_uint64_t c)
  {
    p8 = a;
    p32 = b;
    p64 = c;

    return 0;
  }

  long op_in8_in32_ptr(Test_iface::Rights, l4_uint8_t const &a,
                       l4_uint32_t const &b)
  {
    p8 = a;
    p32 = b;

    return 42;
  }

  long op_o64_o32_o8(Test_iface::Rights, l4_uint64_t &a,
                     l4_uint32_t &b, l4_uint8_t &c)
  {
    a = p64;
    b = p32;
    c = p8;

    return 0;
  }

  long op_o8_o32_o64(Test_iface::Rights, l4_uint8_t &a,
                     l4_uint32_t &b, l4_uint64_t &c)
  {
    a = p8;
    b = p32;
    c = p64;

    return 0;
  }

  l4_uint64_t p64;
  l4_uint32_t p32;
  l4_uint16_t p16;
  l4_uint8_t p8;
};

typedef Atkins::Fixture::Epiface_thread<Test_handler> AlignmentRPC;

TEST_F(AlignmentRPC, In64In32In8)
{
  ASSERT_EQ(0, scap()->in64_in32_in8(45, 87, 1));
  EXPECT_EQ(45U, handler().p64);
  EXPECT_EQ(87U, handler().p32);
  EXPECT_EQ(1U,  handler().p8);
}

TEST_F(AlignmentRPC, In8In32In64)
{
  ASSERT_EQ(0, scap()->in8_in32_in64(222, 689143, 45637346));
  EXPECT_EQ(45637346U, handler().p64);
  EXPECT_EQ(689143U, handler().p32);
  EXPECT_EQ(222U,  handler().p8);
}

TEST_F(AlignmentRPC, In8In32Ptr)
{
  l4_uint8_t a = 73;
  l4_uint32_t b = 9110;
  ASSERT_EQ(42, scap()->in8_in32_ptr(&a, &b));
  EXPECT_EQ(a, handler().p8);
  EXPECT_EQ(b, handler().p32);
}

TEST_F(AlignmentRPC, Out64Out32Out8)
{
  handler().p64 = 5732857;
  handler().p32 = 444888;
  handler().p8  = 45;
  l4_uint8_t i8;
  l4_uint32_t i32;
  l4_uint64_t i64;
  ASSERT_EQ(0, scap()->o64_o32_o8(&i64, &i32, &i8));
  EXPECT_EQ(handler().p64, i64);
  EXPECT_EQ(handler().p32, i32);
  EXPECT_EQ(handler().p8, i8);
}

TEST_F(AlignmentRPC, Out8Out32Out64)
{
  handler().p64 = 67;
  handler().p32 = 1;
  handler().p8  = 200;
  l4_uint8_t i8;
  l4_uint32_t i32;
  l4_uint64_t i64;
  ASSERT_EQ(0, scap()->o8_o32_o64(&i8, &i32, &i64));
  EXPECT_EQ(handler().p64, i64);
  EXPECT_EQ(handler().p32, i32);
  EXPECT_EQ(handler().p8, i8);
}
