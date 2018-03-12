/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
/*
 * Some testing for l4util functions
 *
 * This probably only runs successful on little-endian and 32 bits archs
 *
 *
 * Extend it, if you feel like that!
 */

#include <stdio.h>

#include <l4/util/atomic.h>
#include <l4/util/bitops.h>

#include <l4/atkins/tap/main>

TEST(L4Util, Cmpxchg)
{
  l4_umword_t val = 345;
#if defined(ARCH_x86) || defined(ARCH_amd64)
  l4_uint16_t val2b = 789;
  l4_uint8_t  val1b = 111;
  l4_uint64_t val8b = 0x1234567812345678ULL;
#endif

  EXPECT_TRUE(l4util_cmpxchg(&val, 345, 678));
  EXPECT_EQ(678U, val);

  EXPECT_FALSE(l4util_cmpxchg(&val, 123, 456));

#if defined(ARCH_x86) || defined(ARCH_amd64)
  EXPECT_TRUE(l4util_cmpxchg16(&val2b, 789, 456));
  EXPECT_EQ(456U, val2b);

  EXPECT_FALSE(l4util_cmpxchg16(&val2b, 4, 5));

  EXPECT_TRUE(l4util_cmpxchg8(&val1b, 111, 122));
  EXPECT_EQ(122U, val1b);

  EXPECT_FALSE(l4util_cmpxchg8(&val1b, 9, 2));

  EXPECT_TRUE(l4util_cmpxchg64(&val8b, 0x1234567812345678ULL, 0x8765432198765432ULL));
  EXPECT_EQ(0x8765432198765432ULL, val8b);

  EXPECT_FALSE(l4util_cmpxchg64(&val8b, 9, 2));
#endif
}

TEST(L4Util, SetBit)
{
  l4_umword_t val     = 3;
  l4_umword_t vals[5] = { 0, };

  l4util_set_bit(0, &val);
  EXPECT_EQ(3U, val);

  l4util_set_bit(4, &val);
  EXPECT_EQ(19U, val);

  l4util_set_bit(3 * 8 * sizeof(l4_umword_t) + 25, vals);
  EXPECT_EQ(0x02000000UL, vals[3]);
}

TEST(L4Util, ClearBit)
{
  l4_umword_t val = 16;
  l4_umword_t vals[9] = { 0xffccbbaa, 0xffccbbaa, 0xffccbbaa, 0xffccbbaa,
                          0xffccbbaa, 0xffccbbaa, 0xffccbbaa, 0xffccbbaa,
                          0xffccbbaa };

  l4util_clear_bit(1, &val);
  EXPECT_EQ(16U, val);

  l4util_clear_bit(4, &val);
  EXPECT_EQ(0U, val);

  l4util_clear_bit(7 * 8 * sizeof(l4_umword_t) + 21, vals);
  EXPECT_EQ(0xffccbbaaU, vals[7]);

  l4util_clear_bit(7 * 8 * sizeof(l4_umword_t) + 3, vals);
  EXPECT_EQ(0xffccbba2U, vals[7]);
}

TEST(L4Util, TestBit)
{
  l4_umword_t val = 6;
  l4_umword_t vals[2] = { 0x34, 0x67 };

  EXPECT_TRUE(l4util_test_bit(2, &val));
  EXPECT_FALSE(l4util_test_bit(17, &val));
  EXPECT_TRUE(l4util_test_bit(sizeof(l4_umword_t) * 8, vals));
}

TEST(L4Util, TestAndSetBit)
{
  l4_umword_t val = 8;

  EXPECT_EQ(0, l4util_test_and_set_bit(2, &val));
  EXPECT_EQ(12U, val);
}

TEST(L4Util, TestAndClearBit)
{
  l4_umword_t val = 8;

  EXPECT_EQ(1, l4util_test_and_clear_bit(3, &val));
  EXPECT_EQ(0U, val);
}

TEST(L4Util, TestbitScanReverse)
{
  EXPECT_EQ(4, l4util_bsr(17));
}

TEST(L4Util, TestBitScanForward)
{
  EXPECT_EQ(12, l4util_bsf(0xbc123000));
}

TEST(L4Util, TestFindFirstZeroBit)
{
  l4_umword_t val = 55;
  l4_umword_t vals[] = { l4_umword_t(~0), l4_umword_t(~0),
                         l4_umword_t(~0xf000) };

  EXPECT_EQ(3, l4util_find_first_zero_bit(&val, sizeof(val) * 8));
  EXPECT_EQ(int(2 * 8 * sizeof(l4_umword_t) + 12),
            l4util_find_first_zero_bit(vals, sizeof(vals) * 8));
  EXPECT_GE(l4util_find_first_zero_bit(vals, 64), 64);
  EXPECT_EQ(0, l4util_find_first_zero_bit(vals, 0));
}

