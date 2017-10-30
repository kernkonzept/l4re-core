/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/atkins/tap/main>

static char inbuf[2 * L4_PAGESIZE] __attribute__ ((__aligned__ (L4_PAGESIZE)));
static char outbuf[2 * L4_PAGESIZE] __attribute__ ((__aligned__ (L4_PAGESIZE)));

static void test_cpy(unsigned long inoff, unsigned long outoff, unsigned len)
{
  memset(inbuf, 0, sizeof(inbuf));
  memset(outbuf, 0, sizeof(outbuf));

  for (unsigned i = 0; i < len; ++i)
    inbuf[i+inoff] = i + 1;

  memcpy(outbuf + outoff, inbuf + inoff, len);

  for (unsigned i = 0; i < len; ++i)
    EXPECT_EQ((char)(i + 1), outbuf[i+outoff]);
}

/**
 * Memcpy works with unaligned addresses and across page boundaries
 */
TEST(Memcpy, Unaligned)
{
  for (unsigned long i = L4_PAGESIZE - 16; i < L4_PAGESIZE + 16; ++i)
    for (unsigned long j = L4_PAGESIZE -16; j < L4_PAGESIZE + 16; ++j)
      test_cpy(i, j, 100);
}


