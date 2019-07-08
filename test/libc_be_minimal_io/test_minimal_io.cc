/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Tests that the libc backend no-op implementation can be called and linked
 * from its associated library.
 */

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

#include <l4/atkins/bare/tap_logger>

/**
 * 'write()' can be called and performs no operation with invalid fd.
 */
static void
check_write_noop(Atkins::Bare::Tap_logger &tap)
{
  char const buf[] = "hello from WriteNoop!\n";
  size_t count = sizeof(buf) - 1;

  errno = 0;
  tap.expect(-1 == write(STDIN_FILENO, buf, count),
             "Function is callable and performs no operation with stdin fd.");
  tap.expect(0 < errno, "An error is signalled via errno.");

  errno = 0;
  tap.expect(-1 == write(3, buf, count),
             "Function is callable and performs no operation with invalid fd.");
  tap.expect(0 < errno, "An error is signalled via errno.");
}

/**
 * Minimal IO version of 'write()' accepts stdout and stderr file descriptors.
 */
static void
check_write_stdio(Atkins::Bare::Tap_logger &tap)
{
  char const buf[] = "hello from WriteStdio!\n";
  size_t count = sizeof(buf) - 1;

  tap.expect((ssize_t)count == write(STDOUT_FILENO, buf, count),
             "Function is callable and accepts stdout file descriptor.");

  tap.expect((ssize_t)count == write(STDERR_FILENO, buf, count),
             "Function is callable and accepts stderr file descriptor.");
}

/**
 * 'read()' can be called and performs no operation.
 */
static void
check_read_noop(Atkins::Bare::Tap_logger &tap)
{
  char buf[8];
  size_t count = sizeof(buf);

  errno = 0;
  tap.expect(-1 == read(STDIN_FILENO, buf, count),
             "Function is callable and performs no operation.");
  tap.expect(0 < errno, "An error is signalled via errno.");
}

/**
 * 'lseek()' can be called and performs no operation.
 */
static void
check_lseek_noop(Atkins::Bare::Tap_logger &tap)
{
  off_t offset = 1;

  errno = 0;
  tap.expect(-1 == lseek(STDIN_FILENO, offset, SEEK_SET),
             "Function is callable and performs no operation.");
  tap.expect(0 < errno, "An error is signalled via errno.");
}

/**
 * 'lseek64()' can be called and performs no operation.
 */
static void
check_lseek64_noop(Atkins::Bare::Tap_logger &tap)
{
  off64_t offset = 1;

  errno = 0;
  tap.expect(-1 == lseek64(STDIN_FILENO, offset, SEEK_SET),
             "Function is callable and performs no operation.");
  tap.expect(0 < errno, "An error is signalled via errno.");
}

int
main(int, char **)
{
  Atkins::Bare::Tap_logger tap;
  tap.start();

  check_write_noop(tap);
  check_write_stdio(tap);
  check_read_noop(tap);
  check_lseek_noop(tap);
  check_lseek64_noop(tap);

  return tap.finish();
}
