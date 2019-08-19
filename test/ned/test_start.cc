/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Timo Nicolai <timo.nicolai@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * This file tests the functionality of the Loader:start() function available
 * in Ned-scripts.
 *
 * It depends on the Lua script 'start.cfg' in the same directory.
 */
#include <gtest/gtest.h>

#include <l4/atkins/factory>
#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>
#include <l4/atkins/tap/tap>
#include <l4/atkins/ipc_helper>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/debug>
#include <l4/sys/ipc_gate>
#include <l4/sys/err.h>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

using L4Re::chkcap;
using L4Re::chksys;
using L4Re::Util::make_unique_cap;
using Atkins::Ipc_helper::Default_test_timeout;

/**
 * Magic constants.
 */
enum { Svr_label = 0x51, Svr_label_ok = 0, Svr_label_not_ok = 1,
       Echo_proto = 0x25 };

static l4_msgtag_t
gate_bind_thread_timeout(l4_cap_idx_t ep, l4_cap_idx_t thread, l4_umword_t label,
                         l4_timeout_t timeout)
{
  l4_utcb_t *utcb = l4_utcb();

  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_RCV_EP_BIND_OP;
  m->mr[1] = label;
  m->mr[2] = l4_map_obj_control(0, 0);
  m->mr[3] = l4_obj_fpage(thread, 0, L4_CAP_FPAGE_RWS).raw;

  return l4_ipc_call(ep, utcb, l4_msgtag(L4_PROTO_KOBJECT, 2, 1, 0), timeout);
}

/**
 * Check if a namespace capability has write rights.
 *
 * It is tested whether it is possible to register other capabilities in the
 * namespace through this capability.
 */
static bool
has_permission_w(L4::Cap<L4Re::Namespace> cap)
{
  // Register an invalid dummy capability.
  L4::Ipc::Cap<void> dummy_cap;
  long tmp = cap->register_obj("dummy", dummy_cap);

  if (tmp == -L4_EPERM)
    return false;

  chksys(tmp, "Register a dummy capability in namespace.");
  return true;
}

/**
 * Check if a factory capability has 'special' rights.
 *
 * It is tested whether it is possible to create a namespace using the factory
 * capability.
 */
static bool
has_permission_s(L4::Cap<L4::Factory> cap)
{
  auto dummy_cap =
    Atkins::Factory::cap<L4Re::Namespace>("Allocate dummy capability");

  long err = l4_error(cap->create(dummy_cap.get()));

  if (err == -L4_EPERM)
    return false;

  chksys(err, "Create a dummy namespace.");
  return true;
}

/**
 * Check if a capability has delete rights.
 *
 * First, a copy of the capability is created by mapping it into this task
 * again, deleting the original capability and then checking whether the copy
 * is still present after that (which it should not be if the original
 * capability had delete rights).
 */
static bool
has_permission_d(L4::Cap<void> cap)
{
  L4::Cap<L4::Task> this_task_cap =
    chkcap(L4Re::Env::env()->task(), "Obtain task capability.");

  auto cap_copy = Atkins::Factory::cap<void>("Allocate capability");

  chksys(this_task_cap->map(this_task_cap, cap.fpage(), cap_copy.snd_base()),
         "Map capability into this task again.");

  auto map_err = l4_error(this_task_cap->cap_valid(cap_copy.get()));
  if (map_err <= 0)
    throw L4::Runtime_error(-L4_ENOSYS,
                            "Mapped capability refers to an object.");

  chksys(this_task_cap->delete_obj(cap), "Delete original capability.");

  auto del_tag = this_task_cap->cap_valid(cap_copy.get());
  chksys(del_tag,
         "Obtain validity of capability copy after deletion of original");

  return del_tag.label() != 1;
}

/**
 * Main function of helper thread.
 */
static void
receive_ipc_gate()
{
  // Allocate space for a capability in this thread's buffer-register block.
  // This is necessary because when an IPC gate without server rights tries
  // to bind itself to this thread, this thread must have a capability receive
  // buffer set up to receive the capability included in the payload of the
  // respective bind_thread() call. Otherwise an IPC error will occur.
  auto cap = make_unique_cap<void>();

  // Obtain pointer to buffer register block.
  l4_buf_regs_t *br = l4_utcb_br();
  br->bdr = 0;
  br->br[0] = L4::Ipc::Small_buf(cap.get(), 0).raw();

  l4_umword_t label;
  int err = l4_ipc_error(l4_ipc_wait(l4_utcb(), &label, L4_IPC_NEVER),
                         l4_utcb());

  if (err)
    {
      printf("Await an IPC from main. IPC error: %s (%i)\n",
             l4sys_errtostr(-(L4_EIPC_LO + err)), err);
      return;
    }

  l4_msg_regs_t *mr = l4_utcb_mr();
  mr->mr[0] = (label == Svr_label) ? Svr_label_ok : Svr_label_not_ok;

  l4_msgtag_t tag = l4_msgtag(Echo_proto, 1, 0, 0);

  l4_ipc_send(L4_SYSF_REPLY, l4_utcb(), tag, L4_IPC_SEND_TIMEOUT_0);
}

/**
 * Namespaces can be passed to this task via Loader:start().
 *
 * Namespace definitions given in entries of the 'caps' table contained in the
 * first argument of Loader:start() should be converted into valid namespace
 * capabilities accessible to this task.  All entries of 'caps' whose values
 * are tables mapping strings to capabilities/strings should be interpreted as
 * namespace definitions. For one such example it is verified that a
 * corresponding valid namespace capability is available to this task and that
 * the correct capabilities are returned when the namespace is queried for
 * their corresponding keys. It is also verified that the right error
 * identifier is returned if keys whose values were given as placeholder
 * strings are queried.
 */
TEST(ReceivedCapabilities, ReceivedNamespace)
{
  L4::Cap<L4Re::Namespace> ns_cap =
    L4Re::Env::env()->get_cap<L4Re::Namespace>("test_namespace");

  ASSERT_L4CAP_PRESENT(ns_cap);

  char const *const expected_entries[][2] = {{"name1", "dummy_cap1"},
                                             {"name2", "dummy_cap2"}};

  auto query_cap = Atkins::Factory::cap<void>("Allocate capability");

  for (auto entry : expected_entries)
    {
      char const *ns_name = entry[0];
      char const *cap_name = entry[1];

      ASSERT_L4OK(ns_cap->query(ns_name, query_cap.get()))
        << "Query for capability registered under the name '" << ns_name
        << "'.";

      L4::Cap<void> reference_cap =
        chkcap(L4Re::Env::env()->get_cap<void>(cap_name),
               "Get reference capability for query from environment table.");

      EXPECT_L4CAP_OBJ_EQ(reference_cap, query_cap.get())
        << "Compare capability returned by query with expected capability.";
    }

  EXPECT_EQ(-L4_EAGAIN, ns_cap->query("name_ph", query_cap.get(),
                                      L4Re::Namespace::To_non_blocking))
    << "The namespace contains a placeholder entry for key 'name_ph'.";
}

/**
 * This task's 'rom' capability is read-only.
 *
 * Each task should implicitly be given access to a capability to the 'rom'
 * namespace via its environment table. This capability should have read-only
 * rights.
 */
TEST(ReceivedCapabilities, ReceivedReadonlyRom)
{
  L4::Cap<L4Re::Namespace> rom =
    chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("rom"));

  ASSERT_L4CAP_PRESENT(rom);

  EXPECT_FALSE(has_permission_w(rom)) << "Rom capability is read-only.";
}

/**
 * Capabilities passed to this task have correct write permissions.
 *
 * Capabilities made available to this task via entries of the 'caps' table in
 * the first argument of the Loader:start() function that has spawned this task
 * must have write rights if and only if their 'mode' was set to include them
 * in the corresponding Ned-script.
 * Two namespace capabilities, one with it's mode set to include write rights
 * and another with it's mode set to be read-only are made available to this
 * task and it is verified via has_permission_w() that this task has the
 * corresponding correct access rights to these capabilities.
 */
TEST(ReceivedCapabilities, CorrectWritePermissions)
{
  L4::Cap<L4Re::Namespace> test_w =
    chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("test_w"));
  L4::Cap<L4Re::Namespace> test_no_w =
    chkcap(L4Re::Env::env()->get_cap<L4Re::Namespace>("test_no_w"));

  ASSERT_L4CAP_PRESENT(test_w);
  EXPECT_TRUE(has_permission_w(test_w)) << "Namespace has write permissions.";

  ASSERT_L4CAP_PRESENT(test_no_w);
  EXPECT_FALSE(has_permission_w(test_no_w))
    << "Namespace does not have write permissions.";
}

/**
 * Capabilities passed to this task have correct 'special' permissions.
 *
 * Capabilities made available to this task via entries of the 'caps' table in
 * the first argument of the Loader:start() function that has spawned this task
 * must have 'special' rights if and only if their 'mode' was set to include
 * them in the corresponding Ned-script.
 * Two factory capabilities, one with it's mode set to include special rights
 * and another with it's mode set to be read-only are made available to this
 * task and it is verified via has_permission_s() that this task has the
 * corresponding correct access rights to these capabilities.
 */
TEST(ReceivedCapabilities, CorrectSpecialPermissions)
{
  L4::Cap<L4::Factory> test_s =
    chkcap(L4Re::Env::env()->get_cap<L4::Factory>("test_s"));
  L4::Cap<L4::Factory> test_no_s =
    chkcap(L4Re::Env::env()->get_cap<L4::Factory>("test_no_s"));

  ASSERT_L4CAP_PRESENT(test_s);
  EXPECT_TRUE(has_permission_s(test_s))
    << "Factory has 'special' permissions.";

  ASSERT_L4CAP_PRESENT(test_no_s);
  EXPECT_FALSE(has_permission_s(test_no_s))
    << "Factory does not have 'special' permissions.";
}

/**
 * Capabilities passed to this task have correct delete permissions.
 *
 * Capabilities made available to this task via entries of the 'caps' table in
 * the first argument of the Loader:start() function that has spawned this task
 * must have delete rights if and only if their 'mode' was set to include them
 * in the corresponding Ned-script.
 * Two capabilities of arbitrary type, one with it's mode set to include
 * delete rights and another with it's mode set to be read-only are made
 * available to this task and it is verified via has_permission_d() that this
 * task has the corresponding correct access rights to these capabilities.
 */
TEST(ReceivedCapabilities, CorrectDeletePermissions)
{
  L4::Cap<void> test_d = chkcap(L4Re::Env::env()->get_cap<void>("test_d"));
  L4::Cap<void> test_no_d =
    chkcap(L4Re::Env::env()->get_cap<void>("test_no_d"));

  ASSERT_L4CAP_PRESENT(test_d);
  EXPECT_TRUE(has_permission_d(test_d)) << "Capability has delete permissions.";

  ASSERT_L4CAP_PRESENT(test_no_d);
  EXPECT_FALSE(has_permission_d(test_no_d))
    << "Capability does not have delete permissions.";
}

/**
 * IPC gate capabilities passed to this task via ned only have server rights if
 * they were created using the svr() function.
 *
 * In the Ned-script belonging to this test, a channel is created using
 * Loader:new_channel() and the resulting IPC gate capability as well as a copy
 * of that capability with server permissions (created using the svr()
 * function) are passed to this task by making them entries in the 'caps' table
 * in the first argument to the Loader:start() function that spawns this task.
 * In this test, a new 'echo' thread is created and both IPC gates are bound to
 * it using bind_gate(). The server capability is bound first and it is expected
 * that this succeeds. Subsequently binding the client capability, because it
 * lacks server rights, should send a message to the echo thread instead.
 * The echo thread receives this message and sends a reply indicating whether
 * the message's label is equal to that which was specified when the server
 * capability was bound to the echo thread. In addition an arbitrary identifier
 * is included in this reply's message tag and it is verified that the main
 * thread receives it correctly (i.e. the kernel has not interfered with it).
 */
TEST(ReceivedCapabilities, CorrectServerPermissions)
{
  L4::Cap<L4::Ipc_gate> svr_cap =
    L4Re::Env::env()->get_cap<L4::Ipc_gate>("svr_cap");
  ASSERT_L4CAP_PRESENT(svr_cap) << "Server IPC gate capability present.";

  L4::Cap<L4::Ipc_gate> no_svr_cap =
    L4Re::Env::env()->get_cap<L4::Ipc_gate>("no_svr_cap");
  ASSERT_L4CAP_PRESENT(no_svr_cap) << "Client IPC gate capability present.";

  // Start echo thread.
  std::thread thr = std::thread(receive_ipc_gate);
  L4::Cap<L4::Thread> echo_thr_cap = std::L4::thread_cap(thr);

  // Bind capability with server rights. If the capability has server rights
  // (as expected), this call will bind the corresponding IPC gate to the
  // thread given as the first argument to the function. If the capability does
  // not have server rights, this will block until the timeout hits.
  l4_msgtag_t svr_tag = gate_bind_thread_timeout(
    svr_cap.cap(), echo_thr_cap.cap(), Svr_label, Default_test_timeout);

  ASSERT_L4OK(svr_tag) << "Bind server IPC gate to echo thread";

  // Bind capability without server rights. If the capability does not have
  // server rights (as expected), this call will not bind the client capability
  // to any thread but instead send an IPC message to the thread that the
  // corresponding IPC gate object pointed to by the IPC gate capability with
  // server rights is already bound to. If the capability does have server
  // rights, this function call will succeed without an error but the subsequent
  // EXPECT statements will fail as the expected values are not returned.
  l4_cap_idx_t main_thr_cap = L4Re::Env::env()->main_thread().cap();
  l4_msgtag_t no_svr_tag =
    gate_bind_thread_timeout(no_svr_cap.cap(), main_thr_cap, 0u,
                             Default_test_timeout);

  long echo_proto = no_svr_tag.label();

  l4_umword_t echo_ret = Svr_label_not_ok;
  if (no_svr_tag.words() == 1)
    echo_ret = l4_utcb_mr()->mr[0];

  thr.join();

  ASSERT_L4IPC_OK(no_svr_tag) << "Receive IPC message from echo thread.";

  EXPECT_EQ(Svr_label_ok, echo_ret)
    << "Capability with server rights successfully bound to echo thread.";

  EXPECT_EQ(Echo_proto, echo_proto)
    << "Echo thread IPC message untouched by kernel.";
}
