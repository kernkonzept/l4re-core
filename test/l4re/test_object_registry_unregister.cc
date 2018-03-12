/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test if unregister correctly handles IPC that are still pending
 * for the server.
 *
 */
#include <l4/re/util/object_registry>
#include <l4/util/util.h>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

#include <thread>

struct Test_handler : L4::Irqep_t<Test_handler>
{
  void handle_irq()
  {
    printf("Test_registry called.\n");
  }
};


static void call_thread(L4::Cap<L4::Irq> svr)
{
  l4_msgtag_t tag = l4_ipc_call(svr.cap(), l4_utcb(),
                                l4_msgtag(0, 0, 0, 0), L4_IPC_NEVER);


  EXPECT_EQ(0U, l4_ipc_error(tag, l4_utcb()));
  EXPECT_EQ(-L4_EBADPROTO, l4_error(tag));
}


class SimpleObjectRegistry : public Atkins::Fixture::Server_thread
{
public:
  virtual void SetUp()
  {
    server.registry()->register_obj(&_handler);
  }

  L4::Cap<Test_handler::Interface> scap() const { return _handler.obj_cap(); }

  Test_handler &handler() { return _handler; }

private:
  Test_handler _handler;
};


TEST_F(SimpleObjectRegistry, UnregisterWithPending)
{
  std::thread t(&call_thread, handler().obj_cap());

  // wait for the thread to have the IPC pending
  l4_sleep(100);

  // unregister the handler
  server.registry()->unregister_obj(&handler());

  start_loop();

  t.join();
}
