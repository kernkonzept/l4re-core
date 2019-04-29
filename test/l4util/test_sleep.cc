/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Testing for l4util sleep and micros2l4to functionality.
 */

#include <l4/util/util.h>
#include <l4/atkins/tap/main>

enum
{
  Timeout_ms = 100,
  Timeout_us = 100 * 1000,
};

static bool
operator == (l4_timeout_s a, l4_timeout_s b)
{
  return a.t == b.t;
}

static bool
operator != (l4_timeout_s a, l4_timeout_s b)
{
  return a.t != b.t;
}

/**
 * Calling sleep with non-zero positive value returns control to caller.
 */
TEST(Sleep, ReturnFromTimeout)
{
  l4_sleep(Timeout_ms);
  EXPECT_TRUE(1) << "Sleep with millisec timeout returns control to caller.";
}

/**
 * Calling sleep with zero value returns control to caller.
 */
TEST(Sleep, ReturnFromTimeoutZero)
{
  l4_sleep(0);
  EXPECT_TRUE(1) << "Sleep with 0 millisec timeout returns control to caller.";
}

/**
 * Calling micro sleep with non-zero positive value returns control to caller.
 */
TEST(Usleep, ReturnFromTimeout)
{
  l4_usleep(Timeout_us);
  EXPECT_TRUE(1) << "Sleep with microsec timeout returns control to caller.";
}

/**
 * Calling micro sleep with zero value returns control to caller.
 */
TEST(Usleep, ReturnFromTimeoutZero)
{
  l4_usleep(0);
  EXPECT_TRUE(1) << "Sleep with 0 microsec timeout returns control to caller.";
}

/**
 * `l4util_micros2l4to()` converts zero value to L4_IPC_TIMEOUT_0 constant.
 */
TEST(MicrosecToL4Timeout, TimeoutZero)
{
  EXPECT_EQ(L4_IPC_TIMEOUT_0, l4util_micros2l4to(0))
    << "Timeout of value 0 returns L4_IPC_TIMEOUT_0 constant.";
}

/**
 * `l4util_micros2l4to()` converts all-ones value to L4_IPC_TIMEOUT_NEVER
 * constant.
 */
TEST(MicrosecToL4Timeout, TimeoutNever)
{
  EXPECT_EQ(L4_IPC_TIMEOUT_NEVER, l4util_micros2l4to(~0U))
    << "Timeout of all bits set returns L4_IPC_TIMEOUT_NEVER constant.";
}

const unsigned int timeout_values[] = {
  1U << 0, // values <= 6  hit the 'log2(64) - 7 < 0' path in implementation
  1U << 1,   1U << 2,   1U << 3,   1U << 4,   1U << 5,   1U << 6,   1U << 7,
  1U << 8,   1U << 9,   1U << 10,  1U << 11,  1U << 12,  1U << 13,  1U << 14,
  1U << 15,  1U << 16,  1U << 17,  1U << 18,  1U << 19,  1U << 20,  1U << 21,
  1U << 22,  1U << 23,  1U << 24,  1U << 25,  1U << 26,  1U << 27,  1U << 28,
  1U << 29,  1U << 30,  1U << 31,
  ~0U - 1, // skip ~0U which is L4_IPC_TIMEOUT_NEVER
  ~0U >> 1,  ~0U >> 2,  ~0U >> 3,  ~0U >> 4,  ~0U >> 5,  ~0U >> 6,  ~0U >> 7,
  ~0U >> 8,  ~0U >> 9,  ~0U >> 10, ~0U >> 11, ~0U >> 12, ~0U >> 13, ~0U >> 14,
  ~0U >> 15, ~0U >> 16, ~0U >> 17, ~0U >> 18, ~0U >> 19, ~0U >> 20, ~0U >> 21,
  ~0U >> 22, ~0U >> 23, ~0U >> 24, ~0U >> 25, ~0U >> 26, ~0U >> 27, ~0U >> 28,
  ~0U >> 29, ~0U >> 30,
};

struct TimeoutRange : testing::TestWithParam<unsigned int>
{
};

static INSTANTIATE_TEST_CASE_P(MicrosecToL4Timeout, TimeoutRange,
                               testing::ValuesIn(timeout_values),
                               testing::PrintToStringParamName());
/**
 * `l4util_micros2l4to()` sets valid timeout and converts it back to usec.
 *
 * The error when converting a timeout round trip from usec to L4 timeout back
 * to usec is expected to be within +/- 1% of the original value.
 */
TEST_P(TimeoutRange, TimeoutValid)
{
  unsigned int t = GetParam();
  l4_timeout_s l4to = l4util_micros2l4to(t);

  ASSERT_NE(L4_IPC_TIMEOUT_0, l4to)
    << "Valid timeout does not return L4_IPC_TIMEOUT_0 constant.";
  ASSERT_NE(L4_IPC_TIMEOUT_NEVER, l4to)
    << "Valid timeout does not return L4_IPC_TIMEOUT_NEVER constant.";

  unsigned e = (l4to.t >> 10) & 0x1f;
  unsigned m = (l4to.t & 0x3ff);
  long long error = 100LL * m * (1U << e) / t - 100;

  EXPECT_TRUE((error >= -1) && (error <= 1))
    << "Converting usec to L4 timeout and back to usec is within 1% error.";
}
