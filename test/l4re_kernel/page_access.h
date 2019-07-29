/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Jakub Jermar <jakub.jermar@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Base classes for testing of read-only and non-executable region support in
 * l4re_kernel.
 */

#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/error_helper>
#include <l4/re/env>

#include <l4/sys/exception>
#include <l4/sys/semaphore>
#include <l4/sys/cxx/ipc_epiface>

#include <l4/atkins/l4_assert>
#include <l4/atkins/factory>
#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/ipc_helper>

#include <functional>

enum
{
  Pf_exc_sync = 0x100,
};

/**
 * Class for implementing the exception handler interface.
 */
class Exc_handler : public L4::Epiface_t<Exc_handler, L4::Exception>
{
  static Atkins::Dbg trace()
  {
    return Atkins::Dbg(Atkins::Dbg::Trace, "ExcHdlr");
  }

public:
  /**
   * Handle exceptions triggered by the tests.
   */
  long op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                    L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
  {
    trace().printf("Received exception\n");

    if (l4_utcb_exc_is_pf(&regs))
      {
        l4_addr_t pfa = l4_utcb_exc_pfa(&regs);
        trace().printf("Exception is page fault at address 0x%lx\n", pfa);

        l4_utcb_mr()->mr[0] = pfa;
        l4_ipc_send(L4Re::Env::env()->main_thread().cap(), l4_utcb(),
                    l4_msgtag(Pf_exc_sync, 1, 0, 0), L4_IPC_NEVER);
      }
    else
      trace().printf("Not a page fault exception: Exc value 0x%lx\n",
                   l4_utcb_exc_typeval(&regs));

    return -L4_ENOREPLY;
  }
};

/**
 * Fixture for the page access tests with exception handling.
 *
 * The fixture starts a helper std::thread which in turn executes a test-defined
 * callback.  It is expected that the callback triggers a page fault exception
 * which is then handled by the fixture's exception handler. Information about
 * the exception is propagated back to the test via an IPC message.
 */
class PageAccess : public Atkins::Fixture::Epiface_thread<Exc_handler>
{
  l4_addr_t clean_error(l4_addr_t pfa) { return pfa & ~7UL; }

protected:
  static Atkins::Dbg trace()
  {
    return Atkins::Dbg(Atkins::Dbg::Trace);
  }

public:
  PageAccess()
  {
    sem = Atkins::Factory::kobj<L4::Semaphore>("Allocating a semaphore.");

    std::thread t([this] {
      trace().printf("Going to access the page\n");
      sem->down();
      callback();
      trace().printf("Survived accessing the page\n");
    });

    auto exc_hdlr = scap();
    L4::Thread::Attr attr;
    attr.exc_handler(exc_hdlr);
    L4Re::chksys(std::L4::thread_cap(t)->control(attr),
                 "Set exception handler for the helper thread.");
    // The thread is going to trigger an exception, so we cannot join it
    t.detach();
  }

  /**
   * Wait for the IPC from the exception handler and check the reported PFA.
   *
   * \param pfa  The expected page fault address.
   */
  void wait_for_pfa(l4_addr_t pfa)
  {
    l4_umword_t label;
    auto tag =
      l4_ipc_wait(l4_utcb(), &label, Atkins::Ipc_helper::Default_test_timeout);
    ASSERT_L4IPC_OK(tag) << "There was no IPC error.";
    ASSERT_EQ(tag.label(), Pf_exc_sync) << "The message is expected.";
    ASSERT_GE(tag.words(), 1U) << "The message has enough words.";
    ASSERT_EQ(clean_error(pfa), clean_error(l4_utcb_mr()->mr[0]))
      << "The page fault occured at the expected location";
  }

protected:
  L4Re::Util::Unique_cap<L4::Semaphore> sem;
  std::function<void(void)> callback;
};


