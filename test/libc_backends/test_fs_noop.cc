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
#include <sys/vfs.h>
#include <errno.h>

#include <l4/atkins/tap/main>

/**
 * 'tmpnam()' can be called.
 */
TEST(FsNoop, TmpNam)
{
  char *s = nullptr;

  EXPECT_FALSE(tmpnam(s)) << "Function is callable.";
}

/**
 * 'statfs()' can be called.
 */
TEST(FsNoop, StatFs)
{
  char const *path = "/dummy/path";
  struct statfs buf;

  errno = 0;
  EXPECT_EQ(-1, statfs(path, &buf)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'fstatfs()' can be called.
 */
TEST(FsNoop, FstatFs)
{
  int fd = 1;
  struct statfs buf;

  errno = 0;
  EXPECT_EQ(-1, fstatfs(fd, &buf)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}
