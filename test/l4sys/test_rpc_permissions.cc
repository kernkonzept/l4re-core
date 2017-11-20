/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test automatic check of permissions in RPC.
 */

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, read, (), L4::Ipc::Call_t<L4_CAP_FPAGE_R>);
  L4_INLINE_RPC(long, write, (), L4::Ipc::Call_t<L4_CAP_FPAGE_W>);
  L4_INLINE_RPC(long, read_write, (), L4::Ipc::Call_t<L4_CAP_FPAGE_RW>);
  L4_INLINE_RPC(long, strong, (), L4::Ipc::Call_t<L4_CAP_FPAGE_S>);
  L4_INLINE_RPC(long, read_strong, (), L4::Ipc::Call_t<L4_CAP_FPAGE_RS>);
  L4_INLINE_RPC(long, write_strong, (),
                L4::Ipc::Call_t<L4_CAP_FPAGE_W | L4_CAP_FPAGE_S>);

  typedef L4::Typeid::Rpcs<read_t, write_t, read_write_t,
                           strong_t, read_strong_t, write_strong_t> Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_read(Test_iface::Rights)
  { return 1; }
  long op_write(Test_iface::Rights r)
  { return r.rights & L4_CAP_FPAGE_W; }
  long op_read_write(Test_iface::Rights r)
  { return r.rights & L4_CAP_FPAGE_W; }
  long op_strong(Test_iface::Rights r)
  { return r.rights & L4_CAP_FPAGE_S; }
  long op_read_strong(Test_iface::Rights r)
  { return r.rights & L4_CAP_FPAGE_S; }
  long op_write_strong(Test_iface::Rights r)
  { return r.rights & L4_CAP_FPAGE_S && r.rights & L4_CAP_FPAGE_W; }
};

struct PermissionRPC : Atkins::Fixture::Epiface_thread<Test_handler>
{
  using Perm_cap = L4Re::Util::Unique_cap<Test_iface>;

  Perm_cap get(unsigned int rights)
  {
    auto ret = L4Re::chkcap(L4Re::Util::make_unique_cap<Test_iface>());
    auto task = L4Re::Env::env()->task();

    task->map(task, scap().fpage(rights), ret.snd_base(L4_MAP_ITEM_MAP) | 0xf0);

    return ret;
  }

};


TEST_F(PermissionRPC, Read)
{
  auto cap = get(L4_CAP_FPAGE_R);
  EXPECT_EQ(1, cap->read());
  EXPECT_EQ(-L4_EPERM, cap->write());
  EXPECT_EQ(-L4_EPERM, cap->read_write());
  EXPECT_EQ(-L4_EPERM, cap->strong());
  EXPECT_EQ(-L4_EPERM, cap->read_strong());
  EXPECT_EQ(-L4_EPERM, cap->write_strong());
}

TEST_F(PermissionRPC, ReadWrite)
{
  auto cap = get(L4_CAP_FPAGE_RW);
  EXPECT_EQ(1, cap->read());
  EXPECT_EQ(1, cap->write());
  EXPECT_EQ(1, cap->read_write());
  EXPECT_EQ(-L4_EPERM, cap->strong());
  EXPECT_EQ(-L4_EPERM, cap->read_strong());
  EXPECT_EQ(-L4_EPERM, cap->write_strong());
}

TEST_F(PermissionRPC, Strong)
{
  auto cap = get(L4_CAP_FPAGE_RS);
  EXPECT_EQ(1, cap->read());
  EXPECT_EQ(-L4_EPERM, cap->write());
  EXPECT_EQ(-L4_EPERM, cap->read_write());
  EXPECT_EQ(2, cap->strong());
  EXPECT_EQ(2, cap->read_strong());
  EXPECT_EQ(-L4_EPERM, cap->write_strong());
}

TEST_F(PermissionRPC, WriteStrong)
{
  auto cap = get(L4_CAP_FPAGE_RWS);
  EXPECT_EQ(1, cap->read());
  EXPECT_EQ(1, cap->write());
  EXPECT_EQ(1, cap->read_write());
  EXPECT_EQ(2, cap->strong());
  EXPECT_EQ(2, cap->read_strong());
  EXPECT_EQ(1, cap->write_strong());
}
