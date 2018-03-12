/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/factory>
#include <l4/sys/ipc_gate>
#include <l4/sys/thread>

#include <l4/sys/err.h>
#include <l4/sys/ipc.h>

#include <l4/re/env>
#include <l4/re/error_helper>

#include <l4/atkins/debug>
#include <l4/atkins/factory>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>

#include "recv_thread.h"

// *** static globals ********************************************************

static auto const factory = L4Re::Env::env()->factory();

// *** Base fixture **********************************************************

/**
 * Base class providing a default environment for Ipc_gate test classes.
 */
class IpcGateTest : public testing::TestWithParam<l4_umword_t>
{
public:
  IpcGateTest() : _recv(Recv_thread<2>()) {}

protected:
  l4_umword_t info(L4::Cap<L4::Ipc_gate> cap) const
  {
    l4_umword_t label = 0xbadcaffe;
    EXPECT_L4OK(cap->get_infos(&label));
    return label;
  }

  l4_msgtag_t send_ipc_to(L4::Cap<L4::Ipc_gate> gate,
                          l4_msgtag_t msg = _default_msg)
  {
    return l4_ipc_call(gate.cap(), l4_utcb(), msg, l4_ipc_timeout(0x10, 0x10, 0x10, 0x10));
  }

protected:
  static l4_msgtag_t const _default_msg;

  Recv_thread<2> _recv;

  void chktag(l4_msgtag_t t) { L4Re::chksys(l4_error(t)); }
}; // class IpcGateTest

l4_msgtag_t const IpcGateTest::_default_msg =
  l4_msgtag(0xff0, 0, 0, 0);


// *** Fixture for Tests with an unbound gate ********************************

/**
 *  Test case for an Ipc_gate created without a valid thread, hence an Ipc_gate
 *  object not bound to any thread. This Ipc_gate will be called `unbound`.
 */
class BoundToInvalid : public IpcGateTest
{
public:
  BoundToInvalid()
  {
    _empty_gate_cap = Atkins::Factory::cap<L4::Ipc_gate>();
    L4Re::chksys(factory->create_gate(_empty_gate_cap.get(),
                                      L4::Cap<L4::Thread>(L4_INVALID_CAP),
                                      DEFAULT_LABEL));
  }

protected:
  L4Re::Util::Auto_cap<L4::Ipc_gate>::Cap _empty_gate_cap;
};


// *** bind_thread tests *****************************************************

/**
 * Label value parameterisation.
 * Requirements: lower 2 bits unused;
 *               max length: half of architecture bit length;
 *
 * -1UL:             Maximum value of the type, violating requirements.
 *  0, 0xabc, 0xffc: Valid labels.
 *  0xff3:           Violating lower 2 bits unused requirement.
 *  (~0UL << 2):     Violating max label length requirement.
 */
l4_umword_t const label_array[6] = {0, -1UL, 0xabc, 0xffc, 0xff3, (~0UL << 2)};

static INSTANTIATE_TEST_CASE_P(LabelTest, BoundToInvalid,
                               testing::ValuesIn(label_array));

/**
 * Value-parameterized test for possible Ipc_gate labels. The unbound gate is
 * bound using the labels provided in the INSTANTIATION above.
 */
TEST_P(BoundToInvalid, BindEmptyGateToSvrThread)
{
  l4_umword_t label = GetParam();
  _recv.set_expected_label(label, 0);
  ASSERT_L4OK(_empty_gate_cap->bind_thread(_recv.thread_cap(0), label))
    << "As the gate is unbound, it is expected to successfully bind to the "
       "thread provided.";

  l4_msgtag_t ret = send_ipc_to(_empty_gate_cap.get());
  ASSERT_L4OK(ret)
    << "After a successful binding the IPC should be delivered without error.";
  ASSERT_EQ(PING_VALUE, ret.label())
    << "The thread behind the Ipc_gate is expected to answer with"
       "PING_VALUE, if the label on the IPC message was as expected.";
}

/**
 *  Value-parameterized test for possible Ipc_gate labels provided in the
 *  INSTANTIATION above, together with an invalid capability selector.
 *  All labels are expected to fail due to the invalid capability selector.
 */
TEST_P(BoundToInvalid, BindToInvalidCap)
{
  l4_umword_t label = GetParam();
  auto inv_cap = L4::Cap<L4::Thread>();
  ASSERT_L4ERR(L4_EMSGMISSARG, _empty_gate_cap->bind_thread(inv_cap, label))
    << "The user land is expected to detect the invalid argument and return an "
       "error.";
}


/**
 *  Value-parameterized test for possible Ipc_gate labels provided in the
 *  INSTANTIATION above.
 *  TODO see below.
 */
TEST_P(BoundToInvalid, BindTriggerEperm)
{
  TODO("To trigger EPERM in kernel, the thread to bind the Ipc_gate to must be missing the CS right\n");
  FAIL();
}

/**
 *  Value-parameterized test for possible Ipc_gate labels provided in the
 *  INSTANTIATION above. The gate is bound first to one thread and then rebound
 *  to a different thread.
 *  Both bindings are validated via an IPC message exchange expecting a
 *  pre-specified answer.
 */
TEST_P(BoundToInvalid, BindAndRebind)
{
  auto gate = _empty_gate_cap;
  l4_umword_t label = GetParam();
  _recv.set_expected_label(label, 0);
  chktag(gate->bind_thread(_recv.thread_cap(0), label));
  l4_msgtag_t ret = send_ipc_to(gate.get());
  chktag(ret);
  ASSERT_EQ(PING_VALUE, ret.label())
    << "The thread behind the Ipc_gate is expected to answer with"
       "PING_VALUE, if the label on the IPC message was as expected.";

  // rebind
  L4::Cap<L4::Thread> second_cap = L4Re::chkcap(_recv.thread_cap(1));
  _recv.set_expected_label(label, 1);
  ASSERT_L4OK(gate->bind_thread(second_cap, label));
  ret = send_ipc_to(gate.get());
  ASSERT_L4OK(ret)
    << "After a successful binding the IPC should be delivered without error.";
  ASSERT_EQ(PING_VALUE, ret.label())
    << "The thread behind the Ipc_gate is expected to answer with"
       "PING_VALUE, if the label on the IPC message was as expected.";
}

// *** get_info tests ********************************************************

/**
 * Although not bound to a thread, the unbound gate was created using a label.
 * Querying the info of the unbound gate is expected to provide this label.
 */
TEST_P(BoundToInvalid, EmptyGateInfo)
{
  ASSERT_EQ(DEFAULT_LABEL, info(_empty_gate_cap.get()))
    << "The info of the gate must show the default label passed to "
       "create_gate.";
}

/**
 *  Value-parameterized test for possible Ipc_gate labels provided in the
 *  INSTANTIATION above. After binding the unbound gate, the label should be
 *  the one provided in the binding and the gate info must provided this label.
 */
TEST_P(BoundToInvalid, EmptyBindInfo)
{
  ASSERT_L4OK(_empty_gate_cap->bind_thread(_recv.thread_cap(0), GetParam()));
  ASSERT_EQ(GetParam(), info(_empty_gate_cap.get()))
    << "The info of the gate must show the label provided to bind_thread.";
}

// *** Tests for a gate created with a valid thread **************************

/**
 *  Test case for an Ipc_gate created with a valid thread.
 */
class BoundGateTest : public IpcGateTest
{
public:
  BoundGateTest()
  {
    _bound_gate_cap = Atkins::Factory::cap<L4::Ipc_gate>();
    L4Re::chksys(factory->create_gate(_bound_gate_cap.get(),
                                      _recv.thread_cap(0), GetParam()));
  }

protected:
  L4Re::Util::Auto_cap<L4::Ipc_gate>::Cap _bound_gate_cap;
};

static INSTANTIATE_TEST_CASE_P(LabelTest, BoundGateTest,
                               testing::ValuesIn(label_array));

/**
 * Value-parameterized test for possible Ipc_gate labels provided in the
 * INSTANTIATION above. Validate the binding of the bound gate by sending an
 * IPC.
 */
TEST_P(BoundGateTest, CreateBoundGate)
{
  _recv.set_expected_label(GetParam(), 0);
  l4_msgtag_t ret = send_ipc_to(_bound_gate_cap.get());

  ASSERT_L4OK(ret);
  ASSERT_EQ(PING_VALUE, ret.label());
}

/**
 * Value-parameterized test for possible Ipc_gate labels provided in the
 * INSTANTIATION above. After validating the initial binding of the gate, the
 * thread the gate is bound to is terminated. Further IPC attempts must fail.
 */
TEST_P(BoundGateTest, BindSendTerminateSend)
{
  unsigned thread_no = 0;
  _recv.set_expected_label(GetParam(), thread_no);
  l4_msgtag_t ret = send_ipc_to(_bound_gate_cap.get());

  chktag(ret);
  ASSERT_EQ(PING_VALUE, ret.label());

  _recv.terminate_thread(thread_no);
  ret = send_ipc_to(_bound_gate_cap.get());
  ASSERT_L4ERR(L4_EIPC_LO + L4_IPC_ENOT_EXISTENT, ret);
}

/**
 * Value-parameterized test for possible Ipc_gate labels provided in the
 * INSTANTIATION above. After validating the initial binding of the gate, the
 * gate is bound to anther thread. This binding is again validated via IPC.
 */
TEST_P(BoundGateTest, CreateBoundAndRebind)
{
  l4_umword_t label = GetParam();
  _recv.set_expected_label(label, 0);

  auto gate = _bound_gate_cap;
  l4_msgtag_t ret = send_ipc_to(gate.get());
  chktag(ret);
  ASSERT_EQ(PING_VALUE, ret.label());

  // rebind
  L4::Cap<L4::Thread> second_cap = L4Re::chkcap(_recv.thread_cap(1));
  _recv.set_expected_label(label, 1);
  ASSERT_L4OK(gate->bind_thread(second_cap, label));

  ret = send_ipc_to(gate.get());
  ASSERT_L4OK(ret);
  ASSERT_EQ(PING_VALUE, ret.label());
}

// *** get_info tests ********************************************************

/**
 * Value-parameterized test for possible Ipc_gate labels provided in the
 * INSTANTIATION above. The label value returned by info must be the one
 * provided to the gate creation.
 */
TEST_P(BoundGateTest, CreateBoundGateInfo)
{
  ASSERT_EQ(GetParam(), info(_bound_gate_cap.get()));
}

/**
 * Value-parameterized test for possible Ipc_gate labels provided in the
 * INSTANTIATION above. After rebinding the gate, the label returned by the
 * gate info must be the one set at the rebinding.
 */
TEST_P(BoundGateTest, CreateBoundRebindInfo)
{
  ASSERT_L4OK(_bound_gate_cap->bind_thread(_recv.thread_cap(1), GetParam() + 1));
  ASSERT_EQ(GetParam() + 1, info(_bound_gate_cap.get()));
}

/**
 * Value-parameterized test for possible Ipc_gate labels provided in the
 * INSTANTIATION above. After terminating the thread bound to the gate, the
 * label info of the gate must not change.
 */
TEST_P(BoundGateTest, DeletedThreadGateInfo)
{
  _recv.terminate_thread(0);
  ASSERT_EQ(GetParam(), info(_bound_gate_cap.get()));
}
