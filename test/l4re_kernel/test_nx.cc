/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Jakub Jermar <jakub.jermar@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Tests for non-executable region support.
 *
 * These tests are executed depending on the HWCONFIG setting of the NX
 * variable. For architectures that don't support non-executable pages NX will
 * be false and the tests will not get executed.
 *
 * Support for non-executable pages:
 *
 *     arm:   YES   (starting with v6plus, but see Cortex-A7 erratum 856125)
 *   arm64:   YES
 *     x86:    NO   (need PAE support)
 *  x86_64:   YES
 *    mips:   YES   (starting with R3, but see QEMU bug 1825311)
 */

#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/error_helper>
#include <l4/re/env>

#include <l4/sys/exception>
#include <l4/sys/semaphore>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/cache.h>

#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/atkins/factory>
#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/ipc_helper>

#include <atomic>
#include <functional>

enum
{
  Pf_exc_sync = 0x100,
};

static Atkins::Dbg dbg{2};

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
 * Fixture for the non-execuable page tests with exception handling.
 *
 * The fixture starts a helper std::thread which in turn executes a test-defined
 * callback.  It is expected that the callback triggers a page fault exception
 * which is then handled by the fixture's exception handler. Information about
 * the exception is propagated back to the test via an IPC message.
 */

class NonExecutable : public Atkins::Fixture::Epiface_thread<Exc_handler>
{
protected:
  static Atkins::Dbg trace()
  {
    return Atkins::Dbg(Atkins::Dbg::Trace);
  }

public:
  NonExecutable()
  {
    sem = Atkins::Factory::kobj<L4::Semaphore>("Allocating a semaphore.");

    std::thread t([this] {
      trace().printf("Going to execute non-executable page\n");
      sem->down();
      callback();
      trace().printf("Survived executing non-executable page\n");
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
   * Wait for the IPC from the exception handler.
   *
   * \param[out] pfa  Page fault address as it was recorded by the exception
   *                  handler.
   */
  void wait_for_pfa(l4_addr_t *pfa)
  {
    l4_umword_t label;
    auto tag =
      l4_ipc_wait(l4_utcb(), &label, Atkins::Ipc_helper::Default_test_timeout);
    ASSERT_L4IPC_OK(tag) << "There was no IPC error.";
    ASSERT_EQ(tag.label(), Pf_exc_sync) << "The message is expected.";
    ASSERT_GE(tag.words(), 1U) << "The message has enough words.";
    *pfa = l4_utcb_mr()->mr[0];
  }

protected:
  L4Re::Util::Unique_cap<L4::Semaphore> sem;
  std::function<void(void)> callback;
};

extern "C" void execute_data(void);
extern "C" l4_uint32_t execute_data_size;

/**
 * Data segment is not executable.
 */
TEST_F(NonExecutable, ElfDataIsNotExecutable)
{
  callback = execute_data;
  trace().printf("execute_data = %p\n", execute_data);
  sem->up();

  l4_addr_t pfa;
  wait_for_pfa(&pfa);

  ASSERT_EQ(pfa, (l4_addr_t)execute_data)
    << "The page fault occured at the expected location.";
}

/**
 * Stack of a thread is not executable.
 */
TEST_F(NonExecutable, StackIsNotExecutable)
{
  std::atomic<l4_addr_t> stack_func;

  callback = [&stack_func](void) {
    uint8_t buf[execute_data_size] __attribute__((aligned(4)));
    memcpy(buf, (void *)execute_data, sizeof(buf));

    l4_cache_coherent((l4_addr_t)buf, (l4_addr_t)&buf[sizeof(buf)]);

    stack_func = (l4_addr_t) buf;
    ((void (*)(void))buf)();
  };

  sem->up();

  l4_addr_t pfa;
  wait_for_pfa(&pfa);

  ASSERT_EQ(pfa, stack_func)
    << "The page fault occured at the expected location.";
}

