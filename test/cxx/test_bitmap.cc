/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/bitmap>

#include <l4/atkins/tap/main>

struct TestBitmap : public testing::Test
{
  template <int BITS>
  void clear_buffer(cxx::Bitmap<BITS> &bm)
  {
    memset(bm.bit_buffer(), 0, bm.bit_buffer_bytes(BITS));
  }

  template <int BITS>
  void fill_buffer(cxx::Bitmap<BITS> &bm, int num = BITS)
  {
    for (int i = 0; i < num; ++i)
      bm.set_bit(i);
  }

};

TEST_F(TestBitmap, SetBit)
{
  cxx::Bitmap<18> bm;

  for (int i = 0; i < 18; ++i)
    {
      bm.bit(i, true);
      for (int j = 0; j <= i; ++j)
        EXPECT_TRUE(bm.bit(j));
    }

  for (int i = 17; i >=0; --i)
    {
      bm.bit(i, false);
      for (int j = 0; j < i; ++j)
        EXPECT_TRUE(bm.bit(j));
      for (int j = i; j < 18; ++j)
        EXPECT_FALSE(bm.bit(j));
    }
}

TEST_F(TestBitmap, SetClearBit)
{
  cxx::Bitmap<18> bm;

  for (int i = 0; i < 18; ++i)
    {
      bm.set_bit(i);
      for (int j = 0; j <= i; ++j)
        EXPECT_TRUE(bm[j]);
    }

  for (int i = 17; i >=0; --i)
    {
      bm.clear_bit(i);
      for (int j = 0; j < i; ++j)
        EXPECT_TRUE(bm[j]);
      for (int j = i; j < 18; ++j)
        EXPECT_FALSE(bm[j]);
    }
}

TEST_F(TestBitmap, ScanZeroCharSize)
{
  cxx::Bitmap<8> bm;
  clear_buffer(bm);

  EXPECT_EQ(0, bm.scan_zero(0));

  bm.set_bit(0);
  EXPECT_EQ(1, bm.scan_zero(0));

  bm.set_bit(1);
  bm.set_bit(4);
  EXPECT_EQ(2, bm.scan_zero(0));
  EXPECT_NE(0, bm.scan_zero(4));
  EXPECT_NE(1, bm.scan_zero(4));
  EXPECT_NE(4, bm.scan_zero(4));

  fill_buffer(bm);
  EXPECT_EQ(-1, bm.scan_zero(0));

  fill_buffer(bm, 8 * bm.bit_buffer_bytes(8) - 1);
  EXPECT_EQ(-1, bm.scan_zero(0));
}

TEST_F(TestBitmap, ScanZeroMultiWord)
{
  cxx::Bitmap<256> bm;
  clear_buffer(bm);


  for (int i = 0; i < 256; ++i)
    {
      EXPECT_EQ(i, bm.scan_zero());
      bm.set_bit(i);
    }

  EXPECT_EQ(-1, bm.scan_zero(0));

  bm.clear_bit(112);
  EXPECT_EQ(112, bm.scan_zero());

  bm.clear_bit(9);
  EXPECT_EQ(9, bm.scan_zero());

  bm.clear_bit(10);
  EXPECT_EQ(9, bm.scan_zero());

  bm.clear_bit(0);
  EXPECT_EQ(0, bm.scan_zero());
}
