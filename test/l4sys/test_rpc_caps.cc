/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test capability transmission when marshalling and unmarshalling RPCs.
 */

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_iface>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/sys/kdebug.h>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>

L4Re::Env const *env = L4Re::Env::env();

struct Test_iface
: L4::Kobject_0t<Test_iface, L4::PROTO_ANY, L4::Type_info::Demand_t<1>>
{
  L4_INLINE_RPC(long, in_cap, (bool, L4::Ipc::Cap<void>));
  L4_INLINE_RPC(long, in_opt_cap, (bool, L4::Ipc::Opt<L4::Ipc::Cap<void> >));
  L4_INLINE_RPC(l4_msgtag_t, out_cap, (L4::Ipc::Out<L4::Cap<void> >));
  L4_INLINE_RPC(l4_msgtag_t, out_cap2, (L4::Ipc::Out<L4::Cap<void> >,
                                        L4::Ipc::Out<L4::Cap<void> >));

  typedef L4::Typeid::Rpcs<in_cap_t, in_opt_cap_t, out_cap_t, out_cap2_t> Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_in_cap(Test_iface::Rights, bool release, L4::Ipc::Snd_fpage fpage)
  {
    return handle_in_cap(release, fpage);
  }

  long op_in_opt_cap(Test_iface::Rights, bool release, L4::Ipc::Snd_fpage fpage)
  {
    return handle_in_cap(release, fpage);
  }

  long op_out_cap(Test_iface::Rights, L4::Ipc::Cap<void> &out_cap)
  {
    out_cap = p_cap;
    return 0;
  }

  long op_out_cap2(Test_iface::Rights, L4::Ipc::Cap<void> &out_cap, L4::Ipc::Cap<void> &out_cap2)
  {
    out_cap = p_cap;
    out_cap2 = p_cap2;
    return 0;
  }

private:
  /**
   * Save a capability that comes in via an IPC call on the first allocated
   * receive slot.
   *
   * \param release  If true, then a new capability slot will be allocated for
   *                 the next incomming capability. When false, the next
   *                 capability would overwrite the capability that was just
   *                 received.
   * \param fpage    Received fpage information, will be saved, so that the
   *                 test code can inspect the content later.
   */
  long handle_in_cap(bool release, L4::Ipc::Snd_fpage fpage)
  {
    p_fpage = fpage;
    p_cap = L4::Epiface::server_iface()->rcv_cap<void>(0);

    if (release)
      L4::Epiface::server_iface()->realloc_rcv_cap(0);

    return 0;
  }

public:
  L4::Ipc::Snd_fpage p_fpage;
  L4::Cap<void> p_cap, p_cap2;
};

struct CapRpc
: Atkins::Fixture::Epiface_thread<Test_handler>,
  ::testing::WithParamInterface<unsigned long>
{
  CapRpc() : Epiface_thread(GetParam())
  {}

  /**
   *  Check that the received fpage reports that a capapbility of the correct
   *  type was received.
   */
  void test_valid_cap()
  {
    EXPECT_EQ(GetParam() == 0, handler().p_fpage.cap_received());
    EXPECT_FALSE(handler().p_fpage.id_received());
    EXPECT_EQ(GetParam() != 0, handler().p_fpage.local_id_received());
    EXPECT_TRUE(handler().p_fpage.is_valid());
  }

  /**
   *  Check that the received fpage contains an invalid capability.
   *
   *  This happens when the capability parameter was optional and no
   *  capability was sent.
   */
  void test_invalid_cap()
  {
    EXPECT_FALSE(handler().p_fpage.is_valid());
    EXPECT_FALSE(handler().p_fpage.cap_received());
    EXPECT_FALSE(handler().p_fpage.id_received());
    EXPECT_FALSE(handler().p_fpage.local_id_received());
  }
};

static INSTANTIATE_TEST_CASE_P(LocalVsGlobal, CapRpc,
                               ::testing::Values(L4_RCV_ITEM_LOCAL_ID, 0));

/**
 * A capability with read-write rights is transferred correctly.
 */
TEST_P(CapRpc, SendRwCap)
{
  ASSERT_EQ(0, scap()->in_cap(false, L4::Ipc::make_cap_rw(env->log())));
  test_valid_cap();
  if (GetParam() == 0)
    {
      EXPECT_GT(handler().p_cap.validate().label(), 0);
    }
}

/**
 * An invalid capability is rejected during marshalling.
 */
TEST_P(CapRpc, SendInvalidCap)
{
  ASSERT_EQ(-L4_EMSGMISSARG, scap()->in_cap(false, L4::Cap<void>()));
}

/**
 * A capability that is not present is transferred and appears as
 * not present to the server.
 */
TEST_P(CapRpc, SendEmptyCap)
{
  auto cap = L4Re::Util::make_unique_cap<void>();
  ASSERT_EQ(0, scap()->in_cap(false, cap.get()));
  // the server still receives what looks like a standard cap
  test_valid_cap();
  // but there is nothing behind the cap
  if (GetParam() == 0)
    {
      EXPECT_EQ(0, handler().p_cap.validate().label());
    }
}

/**
 * An invalid capability parameter is ignored if the capability parameter is
 * optional.
 */
TEST_P(CapRpc, SendOptCapInvalidCap)
{
  ASSERT_EQ(L4_EOK, scap()->in_opt_cap(false, L4::Cap<void>()));
  test_invalid_cap();
}

/**
 * An invalid IPC capability parameter is ignored if the capability parameter is
 * optional.
 */
TEST_P(CapRpc, SendOptCapInvalid)
{
  ASSERT_EQ(L4_EOK, scap()->in_opt_cap(false, L4::Ipc::Cap<void>()));
  test_invalid_cap();
}

/**
 * A valid capability is transferred correctly if the capability parameter is
 * optional.
 */
TEST_P(CapRpc, SendOptCapValid)
{
  ASSERT_EQ(0, scap()->in_opt_cap(false, L4::Ipc::make_cap_rw<void>(env->log())));
  test_valid_cap();
  if (GetParam() == 0)
    {
      EXPECT_GT(handler().p_cap.validate().label(), 0);
    }
}

/**
 * A valid capability sent by the server is transferred back correctly.
 */
TEST_P(CapRpc, RcvCap)
{
  auto rcv_cap = L4Re::Util::make_unique_cap<L4Re::Namespace>();
  ASSERT_TRUE(rcv_cap);
  handler().p_cap = env->get_cap<L4Re::Namespace>("rom");
  l4_msgtag_t res = scap()->out_cap(rcv_cap.get());
  ASSERT_EQ(0, l4_error(res));
  ASSERT_EQ(1U, res.items());
  auto test_cap = L4Re::Util::make_unique_cap<void>();
  // test that we really got the rom cap back
  ASSERT_EQ(L4_EOK, rcv_cap->query("l4re", test_cap.get()));
}

/**
 * An invalid capability sent by the server is transferred back correctly.
 */
TEST_P(CapRpc, RcvCapInvalid)
{
  auto rcv_cap = L4Re::Util::make_unique_cap<L4Re::Namespace>();
  ASSERT_TRUE(rcv_cap);
  handler().p_cap = L4::Cap<void>();
  l4_msgtag_t res = scap()->out_cap(rcv_cap.get());
  ASSERT_EQ(0, l4_error(res));
  ASSERT_EQ(1U, res.items());
}

/**
 * If an invalid capability and a valid capability is sent by the server
 * then the valid capability arrives in the expected slot at the client.
 */
TEST_P(CapRpc, RcvCapOneOfTwoInvalid)
{
  auto rcv_cap = L4Re::Util::make_unique_cap<L4Re::Namespace>();
  auto rcv_cap2 = L4Re::Util::make_unique_cap<L4Re::Namespace>();
  ASSERT_TRUE(rcv_cap);
  handler().p_cap = L4::Cap<void>();
  handler().p_cap2 = env->get_cap<L4Re::Namespace>("rom");
  l4_msgtag_t res = scap()->out_cap2(rcv_cap.get(), rcv_cap2.get());
  ASSERT_EQ(0, l4_error(res));
  EXPECT_EQ(2U, res.items());
  EXPECT_L4CAP_NOT_PRESENT(rcv_cap.get());
  EXPECT_L4CAP_OBJ_EQ(rcv_cap2.get(), env->get_cap<L4Re::Namespace>("rom"));
}

struct CapRpcRelease
: Atkins::Fixture::Epiface_thread<Test_handler>,
  ::testing::WithParamInterface<bool>
{
  CapRpcRelease() : Epiface_thread(0)
  {}
};

static INSTANTIATE_TEST_CASE_P(Singleton, CapRpcRelease, ::testing::Bool());

/**
 * When a server reallocates the receive slot after receiving a capability,
 * the next received capability does not overwrite the previous one.
 */
TEST_P(CapRpcRelease, SendCap)
{
  auto cap = env->get_cap<L4Re::Namespace>("rom");

  ASSERT_EQ(0, scap()->in_cap(GetParam(), cap));
  auto outcap = handler().p_cap;
  ASSERT_EQ(0, scap()->in_cap(GetParam(), cap));
  ASSERT_EQ(!GetParam(), outcap.cap() == handler().p_cap.cap());
}
