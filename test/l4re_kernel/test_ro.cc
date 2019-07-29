/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Jakub Jermar <jakub.jermar@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/* Tests for read-only region support. */

#include <l4/sys/semaphore>

#include <l4/atkins/tap/main>

#include "page_access.h"

/** Fixture for the read-only page tests with exception handling. */
using NonWritable = PageAccess;

/**
 * Text segment is not writable.
 */
TEST_F(NonWritable, TextIsNotWritable)
{
  callback = [](void) {
    *(l4_uint8_t *)main = 42;
  };

  sem->up();

  wait_for_pfa((l4_addr_t)main);
}

