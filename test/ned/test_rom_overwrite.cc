/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Timo Nicolai <timo.nicolai@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * This file tests whether the 'rom' namespace capability which is usually
 * implicitely made available to tasks started using
 * Loader:start() or Loader:startv() can be overwritten.
 *
 * It depends on the Lua script 'rom_overwrite.cfg' in the same directory.
 */
#include <gtest/gtest.h>
#include <l4/atkins/factory>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>
#include <l4/re/error_helper>
#include <l4/re/namespace>

/**
 * 'rom' namespace does not contain 'ned' capability.
 */
TEST(RomOverwrite, RomEmpty)
{
  L4::Cap<L4Re::Namespace> rom_cap =
    L4Re::Env::env()->get_cap<L4Re::Namespace>("rom");

  ASSERT_L4CAP_PRESENT(rom_cap) << "'rom' namespace present.";

  auto ned_autocap = Atkins::Factory::cap<L4::Task>();
  auto err = rom_cap->query("ned", ned_autocap.get());
  EXPECT_EQ(-L4_ENOENT, err)
    << "'ned' namespace capability not contained in 'rom' namespace.";
}
