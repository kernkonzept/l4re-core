/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Benjamin Lamowski <benjamin.lamowski@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test Posix file functions of the L4Re libc backend.
 */

#include <errno.h>

#include <l4/atkins/tap/main>

/**
 * The chroot function can be called.
 */
TEST(BeL4ReUnimplemented, Chroot)
{
  char const *path = "/dummy/path";

  errno = 0;
  EXPECT_EQ(-1, chroot(path)) << "Function is callable.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix mkfifo function can be called.
 */
TEST(BeL4ReUnimplemented, Mkfifo)
{
  char const *path = "/dummy/path";
  mode_t mode = 0;

  errno = 0;
  EXPECT_EQ(-1, mkfifo(path, mode)) << "Function is callable.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix chown function can be called.
 */
TEST(BeL4ReUnimplemented, Chown)
{
  char const *path = "/dummy/path";
  uid_t owner = 0;
  gid_t group = 0;

  errno = 0;
  EXPECT_EQ(-1, chown(path, owner, group)) << "Function is callable.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix fchown function can be called.
 */
TEST(BeL4ReUnimplemented, Fchown)
{
  int fd = 0;
  uid_t owner = 0;
  gid_t group = 0;

  errno = 0;
  EXPECT_EQ(-1, fchown(fd, owner, group)) << "Function is callable.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix lchown function can be called.
 */
TEST(BeL4ReUnimplemented, Lchown)
{
  char const *path = "/dummy/path";
  uid_t owner = 0;
  gid_t group = 0;

  errno = 0;
  EXPECT_EQ(-1, lchown(path, owner, group)) << "Function is callable.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}
