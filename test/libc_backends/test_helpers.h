/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Benjamin Lamowski <benjamin.lamowski@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * \file
 * Definitions and helper functions for L4Re libc backend tests.
 */

#pragma once

#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <unistd.h>
#include <sys/fcntl.h>

#include <l4/re/error_helper>

/**
 * Test file paths.
 */
static char const *Ro_test_file = "/rom/l4re_file_testfile_ro";
static char const *Rw_test_file = "/rwfs/l4re_file_testfile_rw";

enum
{
  Ro_test_file_size = 9,
  Rw_test_file_size = 10
};

/**
 * File descriptor that is automatically closed when going out of scope.
 */
class AutoCloseFd
{
public:
  AutoCloseFd(int file_descriptor) : _fd(file_descriptor) {}

  int get() const { return _fd; }

  ~AutoCloseFd()
  {
    // We don't check for errors here because fd may be initialized with invalid
    // file descriptors.
    close(_fd);
  }

  bool operator == (AutoCloseFd const &f) const { return (_fd == f.get()); }
  bool operator != (AutoCloseFd const &f) const { return !(_fd == f.get()); }
  bool operator == (int i) const { return (_fd == i); }
  bool operator >= (int i) const { return (_fd >= i); }

  friend bool operator == (int i, AutoCloseFd const &fd) { return (fd == i); }
  friend bool operator != (int i, AutoCloseFd const &fd) { return !(fd == i); }

private:
  int _fd;
};

/**
 * Return a file descriptor to the read only test file that closes automatically.
 */
inline AutoCloseFd
open_ro_file()
{
  errno = 0;
  int fd = open(Ro_test_file, O_RDONLY);

  if (fd == -1)
      L4Re::chksys(-L4_EINVAL, "Open /rom/l4re_file_testfile_ro.");

  return AutoCloseFd(fd);
}

/**
 * Return a file descriptor to the writeable test file that closes automatically.
 */
inline AutoCloseFd
open_rw_file()
{
  errno = 0;
  int fd = open(Rw_test_file, O_RDWR);

  if (fd == -1)
      L4Re::chksys(-L4_EINVAL, "Open /rom/l4re_file_testfile_rw.");

  return AutoCloseFd(fd);
}

/*
 * Return an invalid file descriptor.
 *
 * We invalidate the file descriptor by closing it.
 */
inline int
invalid_fd()
{
  errno = 0;
  int fd = open(Ro_test_file, O_RDONLY);
  if (fd == -1)
      L4Re::chksys(-L4_EINVAL, "Open /rom/l4re_file_testfile_ro.");

  errno = 0;
  close(fd);
  if (errno > 0)
      L4Re::chksys(-L4_EINVAL, "Close /rom/l4re_file_testfile_ro.");

  return fd;
}

/**
 * Handle partially successful reads of a requested file.
 */
inline ssize_t
successive_read(int fd, char *buf, ssize_t nbyte)
{
  ssize_t bytes_read = 0, result = 0, bytes_remaining = nbyte;

  do
    {
      result = read(fd, buf + bytes_read, bytes_remaining);

      // So far we're not retrying on EINTR but simply fail.
      if (result == -1)
        return -1;

      if (result > bytes_remaining)
        return -1;

      bytes_read += result;
      bytes_remaining -= result;
    }
  while (result > 0);

  return bytes_read;
}
