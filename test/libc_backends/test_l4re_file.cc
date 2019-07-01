/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Benjamin Lamowski <benjamin.lamowski@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * \file
 * Test Posix file functions of the L4Re libc backend.
 *
 * This file relies on the presence of the following test setup:
 * - The supplied file l4re_file_testfile_ro in /rom.
 * - The supplied file l4re_file_testfile_rw writeable in /rwfs.
 * - A writeable namespace /new_rw.
 */
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <errno.h>

#include <sys/param.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <utime.h>

#include <l4/re/error_helper>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>

/**
 * Test file paths.
 */
char const *Ro_test_file = "/rom/l4re_file_testfile_ro";
char const *Rw_test_file = "/rwfs/l4re_file_testfile_rw";

enum
{
  Ro_test_file_size = 9,
  Rw_test_file_size = 10
};

// *** helper functions ********************************************************

/**
 * Helper function to handle partially successful reads of a requested file.
 */
static ssize_t
successive_read(int fd, char *buf, ssize_t nbyte)
{
  ssize_t bytes_read = 0, result = 0, bytes_remaining = nbyte;

  do
    {
      result = read(fd, buf + bytes_read, bytes_remaining);

      // So far we're not retrying on EINTR but simply fail.
      if (result == -1)
        return -1;

      EXPECT_LE(result, bytes_remaining)
        << "Not more bytes than requested have been read.";
      if (result > bytes_remaining)
        return -1;

      bytes_read += result;
      bytes_remaining -= result;
    }
  while (result > 0);

  return bytes_read;
}

/**
 * File descriptor that is automatically closed when going out of scope.
 */
class AutoCloseFd
{
public:
  AutoCloseFd(int file_descriptor) : _fd(file_descriptor) {};

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
static AutoCloseFd
open_ro_file()
{
  errno = 0;
  int fd = open(Ro_test_file, O_RDONLY);

  if (fd == -1)
    {
      Atkins::Dbg().printf("errno: %d\n", errno);
      L4Re::chksys(L4_EINVAL, "Open /rom/l4re_file_testfile_ro.");
    }

  return AutoCloseFd(fd);
}

/**
 * Return a file descriptor to the writeable test file that closes automatically.
 */
static AutoCloseFd
open_rw_file()
{
  errno = 0;
  int fd = open(Rw_test_file, O_RDWR);

  if (fd == -1)
    {
      Atkins::Dbg().printf("errno: %d\n", errno);
      L4Re::chksys(L4_EINVAL, "Open /rom/l4re_file_testfile_rw.");
    }

  return AutoCloseFd(fd);
}

/*
 * Return an invalid file descriptor.
 *
 * We invalidate the file descriptor by closing it.
 */
static int
invalid_fd()
{
  errno = 0;
  int fd = open(Ro_test_file, O_RDONLY);
  if (fd == -1)
    {
      Atkins::Dbg().printf("errno: %d\n", errno);
      L4Re::chksys(L4_EINVAL, "Open /rom/l4re_file_testfile_ro.");
    }

  errno = 0;
  close(fd);
  if (errno > 0)
    {
      Atkins::Dbg().printf("errno: %d\n", errno);
      L4Re::chksys(L4_EINVAL, "Close /rom/l4re_file_testfile_ro.");
    }

  return fd;
}

// *** open ********************************************************************

/**
 * Opening a file with O_RDONLY returns a valid file descriptor.
 */
TEST(BeL4ReOpen, ValidReadOnlyFileDescriptor)
{
  AutoCloseFd fd = open(Ro_test_file, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";
}

/**
 * Opening a directory path without a trailing '/' returns a valid file
 * descriptor.
 */
TEST(BeL4ReOpen, DirectoryPathNoTrailingSlash)
{
  char const *path = "/rom";

  AutoCloseFd fd = open(path, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";
}

/**
 * Opening a directory path with a trailing '/' returns a valid file descriptor.
 */
TEST(BeL4ReOpen, DirectoryPathWithTrailingSlash)
{
  char const *path = "/rom/";

  AutoCloseFd fd = open(path, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";
}

/**
 * Opening a new file with O_CREAT is currently unimplemented and returns an
 * error.
 */
TEST(BeL4ReOpen, CreateNewFileUnimplemented)
{
  char const *path = "/new_rw/new_file";

  errno = 0;
  AutoCloseFd fd = open(path, O_RDWR | O_CREAT);
  ASSERT_EQ(-1, fd) << "Open a previously non-existent file.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * An existing file can be opened with the O_CREAT flag.
 */
TEST(BeL4ReOpen, CreateExistingFile)
{
  AutoCloseFd fd = open(Ro_test_file, O_RDONLY | O_CREAT);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";
}

/**
 * Opening the same file twice results in two distinct file descriptors.
 */
TEST(BeL4ReOpen, DistinctFileDescriptors)
{
  AutoCloseFd fd1 = open(Ro_test_file, O_RDONLY);
  ASSERT_GE(fd1, 0)
    << "The first file descriptor returned by open() is a non-negative number.";
  AutoCloseFd fd2 = open(Ro_test_file, O_RDONLY);
  ASSERT_GE(fd2, 0) << "The second file descriptor returned by open() is a "
                       "non-negative number.";
  ASSERT_NE(fd1, fd2) << "The file descriptors are distinct.";
}

/**
 * Opening a non-existent path returns an error.
 */
TEST(BeL4ReOpen, NonExistentPathError)
{
  char const *path = "/this_certainly_does_not_exist";

  AutoCloseFd fd = open(path, O_RDONLY);
  ASSERT_EQ(-1, fd) << "Open an non-existent path.";
  ASSERT_EQ(ENOENT, errno) << "Errno is set as specified.";
}

// *** open64 ******************************************************************

/**
 * Opening a file using open64() returns a file descriptor.
 */
TEST(BeL4ReOpen64, ReadOnlyFileDescriptor)
{
  AutoCloseFd fd = open64(Ro_test_file, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open64() is a non-negative number.";
}

// *** close *******************************************************************

/**
 * It is possible to close an open file descriptor.
 *
 * This deliberately ignores spurious conditions such as EIO due to errors in
 * the underlying file system, since the setup is supposed to be in working
 * order.
 */
TEST(BeL4ReClose, OpenFile)
{
  int fd = open(Ro_test_file, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";

  ASSERT_EQ(0, close(fd)) << "Closing the open file descriptor is successful.";
}

/**
 * Closing an invalid file descriptor returns an error.
 */
TEST(BeL4ReClose, InvalidFileDescriptor)
{
  errno = 0;
  ASSERT_EQ(-1, close(invalid_fd())) << "Close an invalid file descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

/**
 * Closing an already closed file descriptor returns an error.
 */
TEST(BeL4ReClose, DoubleCloseError)
{
  int fd = open(Ro_test_file, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";

  ASSERT_EQ(0, close(fd)) << "Closing the open file descriptor is successful.";
  errno = 0;
  ASSERT_EQ(-1, close(fd)) << "Closing the already closed file descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

// *** stat ********************************************************************

/**
 * stat() returns the size of the underlying dataspace.
 */
TEST(BeL4ReStat, RoFileInfo)
{
  struct stat buf;
  ASSERT_EQ(0, stat(Ro_test_file, &buf))
    << "Call stat() on the read only testfile.";
  ASSERT_EQ(Ro_test_file_size, buf.st_size)
    << "Compare to the known file size.";
}

// *** llstat *******************************************************************

/**
 * lstat() returns the size of the underlying dataspace.
 */
TEST(BeL4ReLstat, RoFileInfo)
{
  struct stat buf;
  ASSERT_EQ(0, lstat(Ro_test_file, &buf))
    << "Call lstat() on the read only testfile.";
  ASSERT_EQ(Ro_test_file_size, buf.st_size)
    << "Compare to the known file size.";
}

// *** fstat *******************************************************************

/**
 * fstat() returns the size of the underlying dataspace.
 */
TEST(BeL4ReFstat, RoFileInfo)
{
  AutoCloseFd fd = open_ro_file();
  struct stat buf;
  ASSERT_EQ(0, fstat(fd.get(), &buf))
    << "Call fstat() on the read only testfile.";
  ASSERT_EQ(Ro_test_file_size, buf.st_size)
    << "Compare to the known file size.";
}

// *** dup *********************************************************************

/**
 * The dup() function actually duplicates a file descriptor.
 */
TEST(BeL4ReDup, ValidDuplicateFileDescriptor)
{
  AutoCloseFd ro_fd = open_ro_file();
  AutoCloseFd dup_fd = dup(ro_fd.get());
  ASSERT_GE(dup_fd, 0)
    << "The file descriptor returned by dup() is a non-negative number.";
  ssize_t const bufsize = 4;
  char buf[bufsize];
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from original file descriptor.";
  ASSERT_EQ(0, memcmp(buf, "read", bufsize))
    << "First file part read correctly.";
  ASSERT_EQ(bufsize, successive_read(dup_fd.get(), buf, bufsize))
    << "Read from cloned file descriptor.";
  ASSERT_EQ(0, memcmp(buf, "only", bufsize))
    << "Second file part read correctly.";
}

/**
 * Calling dup() on an invalid file descriptor returns an error.
 */
TEST(BeL4ReDup, InvalidFileDescriptor)
{
  errno = 0;
  ASSERT_EQ(-1, dup(invalid_fd())) << "Duplicate an invalid file descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

// *** dup2 ********************************************************************

/**
 * The dup2() function actually duplicates a file descriptor.
 */
TEST(BeL4ReDup2, ValidDuplicateFileDescriptor)
{
  AutoCloseFd ro_fd = open_ro_file();
  AutoCloseFd rw_fd = open_rw_file();
  int dup_fd = dup2(ro_fd.get(), rw_fd.get());
  ASSERT_EQ(dup_fd, rw_fd.get()) << "The second file descriptor is returned.";
  ssize_t const bufsize = 4;
  char buf[bufsize];
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from original file descriptor.";
  ASSERT_EQ(0, memcmp(buf, "read", bufsize))
    << "First file part read correctly.";
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from second file descriptor.";
  ASSERT_EQ(0, memcmp(buf, "only", bufsize))
    << "Second file part read correctly.";
}

/**
 * Calling dup2() with an invalid first file descriptor returns an error.
 */
TEST(BeL4ReDup2, InvalidFileDescriptorAsFirstParameter)
{
  AutoCloseFd ro_fd = open_ro_file();

  ASSERT_EQ(-1, dup2(invalid_fd(), ro_fd.get()))
    << "Duplicate an invalid file descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

/**
 * Calling dup2() with an invalid second file descriptor returns an error.
 */
TEST(BeL4ReDup2, InvalidFileDescriptorAsSecondParameter)
{
  TODO("#1084");
  AutoCloseFd ro_fd = open_ro_file();

  ASSERT_EQ(-1, dup2(ro_fd.get(), invalid_fd()))
    << "Duplicate to an invalid file descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

/**
 * Calling dup2() with two invalid file descriptors results returns an error.
 */
TEST(BeL4ReDup2, BothFileDescriptorsAreInvalid)
{
  int const fd1 = invalid_fd();
  int const fd2 = -21;
  ASSERT_EQ(-1, dup2(fd1, fd2))
    << "Duplicate an invalid file descriptor onto another invalid file "
       "descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

// *** fcntl *******************************************************************

/**
 * Parametrized test setup for valid fcntl commands.
 */
class BeL4ReFcntlFlagsValid : public ::testing::TestWithParam<int> {};
INSTANTIATE_TEST_CASE_P(MultiFlags, BeL4ReFcntlFlagsValid,
                        ::testing::Values(F_GETFD, F_GETFL, F_GETOWN,
                                          F_GETSIG));

/**
 * Calling fcntl() with valid commands on a valid file descriptor returns
 * non-negative flags.
 */
TEST_P(BeL4ReFcntlFlagsValid, ValidFlagsForValidFileDescriptor)
{
  AutoCloseFd ro_fd = open_ro_file();
  ASSERT_GE(fcntl(ro_fd.get(), GetParam()), 0)
    << "Calling fnctl() on a valid file descriptor.";
}

/**
 * Parametrized test setup for unimplemented fcntl commands.
 */
class BeL4ReFcntlFlagsUnimplemented : public ::testing::TestWithParam<int> {};
INSTANTIATE_TEST_CASE_P(MultiFlags, BeL4ReFcntlFlagsUnimplemented,
                        ::testing::Values(F_SETOWN, F_SETSIG));

/**
 * Calling fcntl() with commands not recognized by the backend returns an error.
 */
TEST_P(BeL4ReFcntlFlagsUnimplemented, UnimplementedFlagsForValidFileDescriptor)
{
  AutoCloseFd ro_fd = open_ro_file();
  int third_param = 0;

  errno = 0;
  ASSERT_EQ(-1, fcntl(ro_fd.get(), GetParam(), third_param))
    << "Calling fnctl() on a valid file descriptor.";
  ASSERT_EQ(EINVAL, errno) << "Errno is set as specified.";
}

/**
 * Parametrized test setup for fcntl lock commands.
 */
class BeL4ReFcntlLockFlags : public ::testing::TestWithParam<int> {};
INSTANTIATE_TEST_CASE_P(MultiFlags, BeL4ReFcntlLockFlags,
                        ::testing::Values(F_GETLK, F_SETLK, F_SETLKW, F_SETOWN,
                                          F_SETSIG));

/**
 * Calling fcntl() on a normal file with lock flags returns an error.
 */
TEST_P(BeL4ReFcntlLockFlags, LockFlagsForValidFileDescriptor)
{
  AutoCloseFd ro_fd = open_ro_file();
  struct flock lock;

  errno = 0;
  ASSERT_EQ(-1, fcntl(ro_fd.get(), GetParam(), &lock))
    << "Calling fnctl() on a valid file descriptor.";
  ASSERT_EQ(EINVAL, errno) << "Errno is set as specified.";
}


/**
 * Calling fcntl() on an invalid file descriptor returns an error.
 */
TEST(BeL4ReFcntl, InvalidFileDescriptor)
{
  errno = 0;
  ASSERT_EQ(-1, fcntl(invalid_fd(), F_GETFD))
    << "Calling fnctl() on an invalid file descriptor.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}

// *** read ********************************************************************

/**
 * Partially reading a file returns the correct result.
 */
TEST(BeL4ReRead, PartiallyReadFile)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const bufsize = 7;
  char buf[bufsize];
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Partial read.";
  ASSERT_EQ(0, memcmp(buf, "readonl", bufsize))
    << "Read data is equal to known file content.";
}

/**
 * Fully reading a file returns the correct result.
 */
TEST(BeL4ReRead, FullyReadFile)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const bufsize = 9;
  char buf[bufsize];

  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "File was read completely.";
  ASSERT_EQ(0, memcmp(buf, "readonly\n", bufsize))
    << "Read data is equal to known file content.";
}

/**
 * Reading stops at end of file.
 */
TEST(BeL4ReRead, ReadStopsAtEOF)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const filesize = 9;
  ssize_t const bufsize = filesize + 2;
  char buf[bufsize];
  buf[bufsize - 2] = 'Y';
  buf[bufsize - 1] = 'Z';

  ssize_t read_bytes = successive_read(ro_fd.get(), buf, bufsize);
  ASSERT_LE(read_bytes, filesize)
    << "Not more than the test file's size have been read.";
  ASSERT_EQ(0, memcmp(buf, "readonly\nYZ", bufsize))
    << "Buffer contains file content and canary.";
}

/**
 * Reading zero bytes returns 0 and does not write into the buffer.
 */
TEST(BeL4ReRead, Zero)
{
  AutoCloseFd ro_fd = open_ro_file();
  size_t const bufsize = 2;
  char buf[bufsize];
  buf[0] = 'Y';
  buf[1] = 'Z';

  ASSERT_EQ(0, read(ro_fd.get(), buf, 0)) << "Read returns zero.";
  ASSERT_EQ(0, memcmp(buf, "YZ", bufsize))
    << "Buffer data has not been overwritten.";
}

/**
 * Reading from an invalid file descriptor returns an error.
 */
TEST(BeL4ReRead, InvalidFileDescriptor)
{
  size_t const bufsize = 2;
  char buf[bufsize];
  errno = 0;
  EXPECT_EQ(-1, read(invalid_fd(), buf, 0))
    << "Read on a closed file descriptor.";
  EXPECT_EQ(EBADF, errno) << "Errno indicates a bad file descriptor.";
}

// *** pread *******************************************************************

/**
 * The pread() function reads a file from an offset.
 */
TEST(BeL4RePread, PartiallyReadFileFromOffset)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const bufsize = 4;
  off_t const offset = 4;
  char buf[bufsize];
  ssize_t bytes_read = pread(ro_fd.get(), buf, bufsize, offset);
  ASSERT_GT(bytes_read, 0) << "Read bytes from offset.";
  ASSERT_EQ(0, memcmp(buf, "only", bytes_read)) << "Data was read from offset.";
}

// *** pread64 *****************************************************************

/**
 * The pread64() function reads a file from an offset.
 */
TEST(BeL4RePread64, PartiallyReadFileFromOffset)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const bufsize = 4;
  off_t const offset = 4;
  char buf[bufsize];
  ssize_t bytes_read = pread64(ro_fd.get(), buf, bufsize, offset);
  ASSERT_GT(bytes_read, 0) << "Read bytes from offset.";
  ASSERT_EQ(0, memcmp(buf, "only", bytes_read)) << "Data was read from offset.";
}

// *** lseek *******************************************************************

/**
 * The lseek() function moves an open file descriptors position forward from the
 * beginning.
 */
TEST(BeL4ReLseek, SeekForwardFromBeginning)
{
  AutoCloseFd ro_fd = open_ro_file();
  off_t const offset = 4;
  ASSERT_EQ(offset, lseek(ro_fd.get(), offset, SEEK_SET))
    << "The lseek() function returns the correct position.";
  ssize_t const bufsize = 4;
  char buf[bufsize];
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from newly positioned file.";
  ASSERT_EQ(0, memcmp(buf, "only", bufsize))
    << "Correct data at new file position was returned.";
}

/**
 * The lseek() function moves an open file descriptors position forward from the
 * current position.
 */
TEST(BeL4ReLseek, SeekForwardFromCurrent)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const initial_offset = 2;
  ssize_t const bufsize = 4;
  char buf[bufsize];
  ASSERT_EQ(initial_offset, successive_read(ro_fd.get(), buf, initial_offset))
    << "Reading the first 2 bytes.";

  off_t const offset = 3;
  ASSERT_EQ(initial_offset + offset, lseek(ro_fd.get(), offset, SEEK_CUR))
    << "The lseek() function returns the correct position.";
  EXPECT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from newly positioned file.";
  ASSERT_EQ(0, memcmp(buf, "nly\n", bufsize))
    << "Correct data at new file position was returned.";
}

/**
 * The lseek() function moves an open file descriptors position backwards from
 * the current position.
 */
TEST(BeL4ReLseek, SeekBackwardsFromCurrent)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const initial_offset = 6;
  ssize_t const bufsize = 6;
  char buf[bufsize];
  ASSERT_EQ(initial_offset, successive_read(ro_fd.get(), buf, initial_offset))
    << "Reading the first 6 bytes.";

  off_t const offset = -4;
  ASSERT_EQ(initial_offset + offset, lseek(ro_fd.get(), offset, SEEK_CUR))
    << "The lseek() function returns the correct position.";
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from newly positioned file.";
  ASSERT_EQ(0, memcmp(buf, "adonly", bufsize))
    << "Correct data at new file position was returned.";
}

/**
 * The lseek() function moves an open file descriptors position backwards from
 * the end.
 */
TEST(BeL4ReLseek, SeekBackwardsFromEnd)
{
  AutoCloseFd ro_fd = open_ro_file();
  off_t const offset = -5;
  ASSERT_EQ(9 + offset, lseek(ro_fd.get(), offset, SEEK_END))
    << "The lseek() function returns the correct position.";
  ssize_t const bufsize = 4;
  char buf[bufsize];
  ASSERT_EQ(bufsize, successive_read(ro_fd.get(), buf, bufsize))
    << "Read from newly positioned file.";
  ASSERT_EQ(0, memcmp(buf, "only", bufsize))
    << "Correct data at new file position was returned.";
}

/**
 * It is possible to seek past the end of the file.
 */
TEST(BeL4ReLseek, SeekPastEndOfFile)
{
  AutoCloseFd ro_fd = open_ro_file();
  off_t const offset = 2;
  ASSERT_EQ(9 + offset, lseek(ro_fd.get(), offset, SEEK_END))
    << "The lseek() function returns a position past the EOF.";
}

/**
 * Trying to seek backwards past the beginning of the file returns an error.
 */
TEST(BeL4ReLseek, SeekBackwardsPastBeginningError)
{
  AutoCloseFd ro_fd = open_ro_file();
  ssize_t const initial_offset = 2;
  ssize_t const bufsize = 4;
  char buf[bufsize];
  ASSERT_EQ(initial_offset, successive_read(ro_fd.get(), buf, initial_offset))
    << "Reading the first 2 bytes.";

  off_t const offset = -4;
  errno = 0;
  ASSERT_EQ(-1, lseek(ro_fd.get(), offset, SEEK_CUR))
    << "Seek backwards past the beginning of the file.";
  EXPECT_EQ(EINVAL, errno) << "Errno indicates a bad offset.";
}

/**
 * Seeking in an invalid file descriptor returns an error.
 */
TEST(BeL4ReLseek, InvalidFileDescriptor)
{
  errno = 0;
  ASSERT_EQ(-1, lseek(invalid_fd(), 4, SEEK_SET))
    << "Seek an invalid file descriptor.";
  EXPECT_EQ(EBADF, errno) << "Errno indicates a bad file descriptor.";
}

/**
 * Calling lseek with an invalid whence returns an error.
 *
 * For distinct integers a, b, c: a*a + b*b + c*c is guaranteed to be
 * different from a, b, and c.
 */
TEST(BeL4ReLseek, InvalidWhence)
{
  AutoCloseFd ro_fd = open_ro_file();
  errno = 0;
  ASSERT_EQ(-1, lseek(ro_fd.get(), 4,
                      SEEK_CUR * SEEK_CUR + SEEK_END * SEEK_END
                        + SEEK_SET * SEEK_SET))
    << "Call lseek() with an invalid whence.";
  EXPECT_EQ(EINVAL, errno) << "Errno indicates a bad whence.";
}

// *** getcwd ******************************************************************

/**
 * The Posix getcwd() function returns an error or a terminated string in a
 * pre-supplied buffer.
 */
TEST(BeL4ReGetcwd, ValidResultInPreAllocatedBuffer)
{
  char buf[MAXPATHLEN];
  errno = 0;

  char const *bufptr = getcwd(buf, MAXPATHLEN);

  if (bufptr == nullptr)
    {
      EXPECT_GT(errno, 0)
        << "Errno is set if the function returns a null pointer.";
    }
  else
    {
      ASSERT_NE(nullptr, memchr(buf, '\0', MAXPATHLEN))
        << "Supplied buffer contains null-terminated string.";
    }
}

/**
 * The Posix getcwd() function allocates a buffer with supplied size and returns
 * a null terminated string.
 *
 * This behaviour is a glibc extension when the buf argument is 0 and a size is
 * supplied.
 */
TEST(BeL4ReGetcwd, ValidStringInAllocatedBuffer)
{
  errno = 0;
  char *bufptr = getcwd(0, MAXPATHLEN);

  if (bufptr == nullptr)
    {
      EXPECT_GT(errno, 0)
        << "Errno is set if the function returns a null pointer.";
    }
  else
    {
      EXPECT_NE(nullptr, memchr(bufptr, '\0', MAXPATHLEN))
        << "Supplied buffer contains null-terminated string.";
      free(bufptr);
    }
}

/**
 * The Posix getcwd() function allocates a dynamically sized buffer.
 *
 * This behaviour is a glibc extension when both arguments are 0.
 * Unfortunately there is no reliable way to validate the returned pointer.
 */
TEST(BeL4ReGetcwd, PointerToAllocatedBuffer)
{
  errno = 0;

  char *bufptr = getcwd(0, 0);

  if (bufptr == nullptr)
    {
      EXPECT_GT(errno, 0)
        << "Errno is set if the function returns a null pointer.";
    }
  else
    free(bufptr);
}

/**
 * The Posix getcwd() function returns an error when a buffer is supplied
 * without a size.
 */
TEST(BeL4ReGetcwd, WithBufferAndNullSize)
{
  char buf[MAXPATHLEN];
  errno = 0;

  EXPECT_EQ(nullptr, getcwd(buf, 0)) << "The function returns a null pointer.";
  EXPECT_EQ(EINVAL, errno) << "Error code is set as specified.";
}

/**
 * The Posix getcwd() function returns the correct result.
 */
TEST(BeL4ReGetcwd, CorrectlyValidateCwd)
{
  char const *newpath = "/rom";
  char const newpathlen = strlen(newpath) + 1;

  char origdir_buf[MAXPATHLEN];
  char const *orig_cwd = getcwd(origdir_buf, MAXPATHLEN);
  ASSERT_NE(nullptr, orig_cwd)
    << "The getcwd() function returns a base directory.";
  ASSERT_EQ(0, chdir(newpath)) << "Changing to a known directory.";

  char chdir_buf[newpathlen];
  char const *cwd = getcwd(chdir_buf, newpathlen);
  ASSERT_NE(nullptr, cwd) << "A working directory is returned.";
  EXPECT_EQ(0, strcmp(newpath, cwd))
    << "Correct working directory is returned.";

  chdir(orig_cwd);
}

// *** chdir *******************************************************************

/**
 * The Posix chdir() function indicates errors on a non-existent path.
 */
TEST(BeL4ReChdir, InvalidPath)
{
  errno = 0;
  char const *path = "/this_certainly_does_not_exist";
  ASSERT_EQ(-1, chdir(path)) << "Changing to a non-existent directory.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix chdir() function changes to a relative path.
 */
TEST(BeL4ReChdir, RelativePath)
{
  char origdir_buf[MAXPATHLEN];
  char const *orig_cwd = getcwd(origdir_buf, MAXPATHLEN);
  ASSERT_NE(nullptr, orig_cwd)
    << "The getcwd() function returns a base directory.";

  ASSERT_EQ(0, chdir("/")) << "Changing to the file system root.";

  char const *relative_path = "rom";
  ASSERT_EQ(0, chdir(relative_path)) << "Changing to a relative path.";

  char const *returned_path = "/rom";
  char const returned_path_len = strlen(returned_path) + 1;

  char chdir_buf[returned_path_len];
  char const *cwd = getcwd(chdir_buf, returned_path_len);
  ASSERT_NE(nullptr, cwd) << "A working directory is returned.";
  EXPECT_EQ(0, strcmp(returned_path, cwd))
    << "Correct working directory is returned.";

  AutoCloseFd fd = open("l4re_file_testfile_ro", O_RDONLY);
  EXPECT_NE(0, fd) << "Open a file relative to the changed directory.";

  chdir(orig_cwd);
}


// *** fchdir ******************************************************************

/**
 * The Posix fchdir() function succeeds to change to a valid file descriptor.
 */
TEST(BeL4ReFchdir, ChangeToValidFileDescriptor)
{
  char const *newpath = "/rom";
  AutoCloseFd fd = open(newpath, O_RDONLY);
  ASSERT_GE(fd, 0)
    << "The file descriptor returned by open() is a non-negative number.";

  char origdir_buf[MAXPATHLEN];
  char const *orig_cwd = getcwd(origdir_buf, MAXPATHLEN);
  ASSERT_NE(nullptr, orig_cwd)
    << "The getcwd() function returns a base directory.";
  ASSERT_EQ(0, fchdir(fd.get())) << "Changing to a known directory.";

  chdir(orig_cwd);
}

/**
 * The Posix fchdir() function indicates errors on an invalid file descriptor.
 */
TEST(BeL4ReFchdir, InvalidFileDescriptor)
{
  errno = 0;
  ASSERT_EQ(-1, fchdir(invalid_fd()))
    << "Changing to an invalid directory handle.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

// *** readdir *****************************************************************

/**
 * The Posix readdir() function returns a directory entry.
 *
 * This only works on namespaces that provide directory contents in .dirinfo.
 */
TEST(BeL4ReReaddir, GetEntry)
{
  char const *dirpath = "/rom";
  DIR *dirp;
  dirp = opendir(dirpath);
  ASSERT_NE(nullptr, dirp) << "Open a directory path.";

  struct dirent *entry;
  char const *known_file = "l4re_file_testfile_ro";
  while ((entry = readdir(dirp)))
    if (!strcmp(known_file, entry->d_name))
      break;

  ASSERT_NE(nullptr, entry) << "Read an entry for a known file.";
}

// *** unimplemented functions *************************************************

/**
 * The chroot() function can be called.
 */
TEST(BeL4ReUnimplemented, Chroot)
{
  char const *path = "/dummy/path";

  errno = 0;
  EXPECT_EQ(-1, chroot(path)) << "Function is callable.";
  EXPECT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix mkfifo() function can be called.
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
 * The Posix chown() function can be called.
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
 * The Posix fchown() function can be called.
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
 * The Posix lchown() function can be called.
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

/**
 * The Posix truncate() function can be called.
 */
TEST(BeL4ReUnimplemented, Truncate)
{
  off_t length = 1;

  EXPECT_EQ(-1, truncate(Ro_test_file, length)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The truncate64() function can be called.
 */
TEST(BeL4ReUnimplemented, Truncate64)
{
  off_t length = 1;

  EXPECT_EQ(-1, truncate64(Ro_test_file, length)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix ftruncate() function can be called.
 */
TEST(BeL4ReUnimplemented, Ftruncate)
{
  AutoCloseFd fd = open_ro_file();
  off_t length = 1;

  EXPECT_EQ(-1, ftruncate(fd.get(), length)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The ftruncate64() function can be called.
 */
TEST(BeL4ReUnimplemented, Ftruncate64)
{
  AutoCloseFd fd = open_ro_file();
  off_t length = 1;

  EXPECT_EQ(-1, ftruncate64(fd.get(), length)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix lockf() function can be called.
 */
TEST(BeL4ReUnimplemented, Lockf)
{
  AutoCloseFd fd = open_ro_file();
  off_t length = 1;

  EXPECT_EQ(-1, lockf(fd.get(), F_TLOCK, length)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix mknod() function can be called.
 */
TEST(BeL4ReUnimplemented, Mknod)
{
  char const *path = "/dummy/path";
  EXPECT_EQ(-1, mknod(path, 0, 0)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix fsync() function can be called.
 */
TEST(BeL4ReUnimplemented, Fsync)
{
  AutoCloseFd fd = open_ro_file();
  EXPECT_EQ(-1, fsync(fd.get())) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix fsync() function can be called.
 */
TEST(BeL4ReUnimplemented, Fdatasync)
{
  AutoCloseFd fd = open_ro_file();
  EXPECT_EQ(-1, fdatasync(fd.get())) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix utime() function can be called.
 */
TEST(BeL4ReUnimplemented, Utime)
{
  EXPECT_EQ(-1, utime(Rw_test_file, 0)) << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix rename() function can be called.
 */
TEST(BeL4ReUnimplemented, Rename)
{
  EXPECT_EQ(-1, rename(Rw_test_file, "/rwfs/new_name"))
   << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}

/**
 * The Posix link() function can be called.
 */
TEST(BeL4ReUnimplemented, Link)
{
  EXPECT_EQ(-1, link(Rw_test_file, "/rwfs/new_name"))
    << "Function is callable.";
  ASSERT_GT(errno, 0) << "Errno is set.";
}
