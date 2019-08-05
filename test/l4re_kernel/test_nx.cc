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

#include <l4/sys/semaphore>
#include <l4/sys/cache.h>

#include <l4/atkins/tap/main>

#include <atomic>

#include "page_access.h"

/** Fixture for the non-execuable page tests with exception handling. */
using NonExecutable = PageAccess;

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

  wait_for_pfa((l4_addr_t)execute_data);
}

/**
 * Stack of a thread is not executable.
 */
TEST_F(NonExecutable, StackIsNotExecutable)
{
  std::atomic<l4_addr_t> stack_func;
  auto cb = Atkins::Factory::kobj<L4::Semaphore>("Allocating a semaphore.");

  callback = [&stack_func, &cb](void) {
    uint8_t buf[execute_data_size] __attribute__((aligned(4)));
    memcpy(buf, (void *)execute_data, sizeof(buf));

    l4_cache_coherent((l4_addr_t)buf, (l4_addr_t)&buf[sizeof(buf)]);

    stack_func = (l4_addr_t) buf;
    cb->up();
    ((void (*)(void))buf)();
  };

  sem->up();
  cb->down();

  wait_for_pfa(stack_func);
}

