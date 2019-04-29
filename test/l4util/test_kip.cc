/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Testing for l4util KIP functionality.
 */

#include <l4/util/kip.h>

#include <l4/atkins/tap/main>

enum
{
  Expected_abi_version = 3UL,
};

static char const *expected_kip_feature = "abiver:3";
static char const *junk_kip_feature = "junkabcdef";

/**
 * `l4util_kip_kernel_abi_version()` returns the expected ABI version.
 */
TEST(L4UtilKip, AbiVersion)
{
  EXPECT_EQ(Expected_abi_version, l4util_kip_kernel_abi_version(l4re_kip()))
    << "Reading the KIP ABI version presents the expected version.";
}

/**
 * `l4util_kip_kernel_has_feature()` can find a feature known to be in the KIP.
 */
TEST(L4UtilKip, KipFeatureContainsExpectedFeature)
{
  EXPECT_TRUE(l4util_kip_kernel_has_feature(l4re_kip(), expected_kip_feature))
    << "KIP contains feature it is known to always contain.";
}

/**
 * `l4util_kip_kernel_has_feature()` fails to find a non-existent feature.
 */
TEST(L4UtilKip, KipFeatureDoesntContainJunkFeature)
{
  EXPECT_FALSE(l4util_kip_kernel_has_feature(l4re_kip(), junk_kip_feature))
    << "Searching KIP features for non-existent feature returns false.";
}

/**
 * `l4util_kip_kernel_has_feature()` returns false when searching for empty
 * string.
 */
TEST(L4UtilKip, KipFeatureSearchForEmptyString)
{
  EXPECT_FALSE(l4util_kip_kernel_has_feature(l4re_kip(), ""))
    << "Searching KIP features with empty string returns false.";
}
