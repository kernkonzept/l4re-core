/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */
#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/dataspace>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>

using L4Re::chksys;
using L4Re::chkcap;
using L4Re::Util::Unique_cap;

auto *env = L4Re::Env::env();

class Rw_module: public ::testing::Test
{
public:
  void SetUp() override
  {
    test_ns = chkcap(env->get_cap<L4Re::Namespace>("testns"),
                     "discover 'testns' namespace");
  }

  L4::Cap<L4Re::Namespace> test_ns;
};

TEST_F(Rw_module, WriteToModule)
{
  auto ds = chkcap(L4Re::Util::make_unique_cap<L4Re::Dataspace>(), "cap alloc");

  ASSERT_L4OK(test_ns->query("rw_module", ds.get()))
    << "find 'rw_module.cfg' in testns";

  L4Re::Dataspace::Stats s;
  ASSERT_L4OK(ds->info(&s))
    << "ds info";

  ASSERT_EQ(L4Re::Dataspace::Map_rw, (s.flags & L4Re::Dataspace::Map_rw))
    << "ds writable";

  L4Re::Rm::Unique_region<void *> base;
  ASSERT_L4OK(env->rm()->attach(&base, s.size, L4Re::Rm::Search_addr,
                                L4::Ipc::make_cap_rw(ds.get())))
    << "attach ds to rm";

  memset(base.get(), 1, s.size);
}
