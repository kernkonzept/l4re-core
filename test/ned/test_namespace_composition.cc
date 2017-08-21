/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test access to Ned's namespace hierarchy.
 */
#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/error_helper>
#include <l4/re/namespace>
#include <l4/re/dataspace>
#include <l4/re/env>

using L4Re::chkcap;
using L4Re::Util::Unique_cap;

class NamespaceComposition: public ::testing::Test
{
public:
  void SetUp() override
  {
    rom_ns = chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("rom"),
                    "discover 'rom' namespace");
    test_ns = chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("testns"),
                     "discover 'testns' namespace");
  }

  L4::Cap<L4Re::Namespace> rom_ns, test_ns;
};

/**
 * A Ned namespace provides access by name to namespace members only.
 */
TEST_F(NamespaceComposition, CheckRomNs)
{
  auto ds = chkcap(L4Re::Util::make_unique_cap<L4Re::Dataspace>(), "cap alloc");

  ASSERT_L4OK(rom_ns->query("test1", ds.get()))
    << "find 'test1' in 'rom'";

  ASSERT_L4CAP(ds.get())
    << "'test1' in 'rom' is a valid ds";

  ASSERT_L4ERR(L4_ENOENT, rom_ns->query("ned", ds.get()))
    << "'ned' need not be visible in 'rom' namespace";
}

/**
 * A Ned namespace provides access to a local member which overlays a global
 * name, but prevents access to global names.
 */
TEST_F(NamespaceComposition, CheckTestNs)
{
  Unique_cap<L4Re::Dataspace> ds =
    chkcap(L4Re::Util::make_unique_cap<L4Re::Dataspace>(), "cap alloc");

  ASSERT_L4OK(test_ns->query("l4re", ds.get()))
    << "find 'l4re' in 'testns'";

  ASSERT_L4CAP(ds.get())
    << "'l4re' in 'testns' is a valid cap";

  ASSERT_L4ERR(L4_ENOENT, test_ns->query("moe", ds.get()))
    << "'moe' must not be visible in 'testns'";
}

