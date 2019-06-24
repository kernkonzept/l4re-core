/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Benjamin Lamowski <benjamin.lamowski@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * \file
 * Test Posix memory map functions of the L4Re libc backend.
 */

#include <sys/mman.h>

#include "test_helpers.h"

#include <l4/atkins/tap/main>

// *** mmap ********************************************************************

/**
 * A dataspace can be mapped into memory completely.
 */
TEST(BeL4ReMmap, FullPrivateMapping)
{
  AutoCloseFd fd = open_ro_file();

  void *buf = mmap(0, Ro_test_file_size, PROT_READ, MAP_PRIVATE, fd.get(), 0);
  ASSERT_NE(MAP_FAILED, buf) << "Map file into memory.";
  EXPECT_EQ(0, memcmp(buf, "readonly\n", Ro_test_file_size))
    << "Mapped memory address contains the file content.";
  ASSERT_EQ(0, munmap(buf, Ro_test_file_size)) << "Unmap the buffer";
}

/**
 * Partially map a file region into memory with an unaligned offset.
 */
TEST(BeL4ReMmap, UnalignedOffset)
{
  AutoCloseFd fd = open_ro_file();
  off_t const unaligned_offset = 4;

  errno = 0;
  ASSERT_EQ(MAP_FAILED,
            mmap(0, 1, PROT_READ, MAP_PRIVATE, fd.get(), unaligned_offset))
    << "Map file at offset unaligned to page size.";
  ASSERT_EQ(EINVAL, errno) << "Errno is set as specified.";
}

/**
 * A private memory mapping is writeable even when backed by a readonly file.
 */
TEST(BeL4ReMmap, PrivateWrite)
{
  AutoCloseFd fd = open_ro_file();

  void *buf = mmap(0, Ro_test_file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
                   fd.get(), 0);
  ASSERT_NE(MAP_FAILED, buf) << "Map file into memory.";

  memcpy(buf, "READ", 4);
  ASSERT_EQ(0, memcmp(buf, "READonly\n", Ro_test_file_size))
    << "Read back written content.";
  ASSERT_EQ(0, munmap(buf, Ro_test_file_size)) << "Unmap the buffer";
}

/**
 * Anonymous memory can be mapped, written to and read back.
 */
TEST(BeL4ReMmap, MapAnonymous)
{
  char const *text = "We are Anonymous. Expect us.";
  size_t textlen = strlen(text) + 1;

  void *buf = mmap(0, textlen, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  ASSERT_NE(MAP_FAILED, buf) << "Map anonymous file.";

  EXPECT_NE(nullptr, strcpy((char *)buf, text))
    << "Write into the mapped memory.";
  EXPECT_EQ(0, memcmp(buf, text, textlen))
    << "The mapped region contains the written content.";
  ASSERT_EQ(0, munmap(buf, textlen)) << "Unmap the buffer";
}

/**
 * Anonymous memory can be mapped over more than L4_PAGESIZE.
 */
TEST(BeL4ReMmap, MapAnonymousPageSize)
{
  char const fillbyte = 7;
  char *buf = (char *)mmap(0, L4_PAGESIZE + 1, PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS, -1, 0);
  ASSERT_NE(MAP_FAILED, buf) << "Map anonymous file.";

  ASSERT_NE(nullptr, memset(buf, fillbyte, L4_PAGESIZE + 1))
    << "Write into the mapped memory.";
  for (size_t pos = 0; pos < (L4_PAGESIZE + 1); ++pos)
    {
      EXPECT_EQ(fillbyte, buf[pos])
        << "The mapped region contains the expected byte.";
    }
  ASSERT_EQ(0, munmap(buf, L4_PAGESIZE + 1)) << "Unmap the buffer";
}

/**
 * Parametrized test setup for location and L4 mmap reserve flags.
 */
class BeL4ReMmapFlags : public ::testing::TestWithParam<std::tuple<int, int>>
{
};
INSTANTIATE_TEST_CASE_P(Flags, BeL4ReMmapFlags,
                        ::testing::Combine(::testing::Values(0, 0x1000000),
                                           ::testing::Values(0, MAP_FIXED)));

/**
 * Memory can be mapped at a reserved location.
 *
 * This tests with and without MAP_FIX and the special L4 flag 0x1000000 for
 * mmap to only reserve memory instead of actually mapping it.
 */
TEST_P(BeL4ReMmapFlags, MapReserve)
{
  int reserve_flags, location_flags;
  std::tie(reserve_flags, location_flags) = GetParam();
  void *addr =
    mmap(0, Ro_test_file_size, PROT_READ, MAP_ANONYMOUS | reserve_flags, -1, 0);
  ASSERT_NE(MAP_FAILED, addr) << "Get an address that is guaranteed to work.";

  AutoCloseFd fd = open_ro_file();
  void *buf = mmap(addr, Ro_test_file_size, PROT_READ,
                   MAP_PRIVATE | location_flags, fd.get(), 0);
  ASSERT_NE(MAP_FAILED, buf) << "Map file into previously reserved memory.";
  EXPECT_EQ(0, memcmp(buf, "readonly\n", Ro_test_file_size))
    << "Mapped memory address contains the file content.";
  ASSERT_EQ(0, munmap(buf, Ro_test_file_size));
}

/**
 * Mapping into memory from an invalid file descriptor returns an error.
 */
TEST(BeL4ReMmap, InvalidFileDescriptor)
{
  errno = 0;
  ASSERT_EQ(MAP_FAILED, mmap(0, 1, PROT_READ, MAP_PRIVATE, invalid_fd(), 0))
    << "Map an invalid file descriptor into memory.";
  ASSERT_EQ(EBADF, errno) << "Errno is set as specified.";
}


/**
 * Mapping fails when length is zero.
 */
TEST(BeL4ReMmap, ZeroLength)
{
  AutoCloseFd fd = open_ro_file();
  errno = 0;
  ASSERT_EQ(MAP_FAILED, mmap(0, 0, PROT_READ, MAP_PRIVATE, fd.get(), 0))
    << "Map a file descriptor with zero length.";
  ASSERT_EQ(EINVAL, errno) << "Errno is set as specified.";
}

// *** mremap ******************************************************************

std::tuple<size_t, size_t> size_combinations[] = {
  std::make_tuple(Ro_test_file_size, Ro_test_file_size - 2),
  std::make_tuple(Ro_test_file_size - 2, Ro_test_file_size),
  std::make_tuple(Ro_test_file_size, L4_PAGESIZE + 1)};

/**
 * Parametrized test setup for mremap tests.
 */
class BeL4ReMremap
: public ::testing::TestWithParam<std::tuple<std::tuple<size_t, size_t>, int>> {};
INSTANTIATE_TEST_CASE_P(
  Flags, BeL4ReMremap,
  ::testing::Combine(::testing::ValuesIn(size_combinations),
                     ::testing::Values(0, MREMAP_MAYMOVE)));

/**
 * Remapping an existing mapping to smaller, bigger and above page sizes may
 * succeed in place and succeeds for sure when the buffer may be moved.
 */
TEST_P(BeL4ReMremap, Resize)
{
  size_t old_size, new_size;
  std::tuple<size_t, size_t> sizes;
  int remap_flags;
  std::tie(sizes, remap_flags) = GetParam();
  std::tie(old_size, new_size) = sizes;

  AutoCloseFd fd = open_ro_file();
  void *orig_buf = mmap(0, old_size, PROT_READ, MAP_PRIVATE, fd.get(), 0);
  ASSERT_NE(MAP_FAILED, orig_buf) << "Map file into memory.";

  void *new_buf = mremap(orig_buf, old_size, new_size, remap_flags);

  if (remap_flags == MREMAP_MAYMOVE)
    {
      ASSERT_NE(MAP_FAILED, new_buf)
        << "Changing the size does not fail when the buffer can be moved.";
    }

  if (new_buf != MAP_FAILED)
    {
      EXPECT_EQ(0, memcmp(new_buf, "readonly\n",
                          (new_size < old_size) ? new_size : old_size))
        << "Mapped memory address contains the file content.";
    }

  if (new_buf == MAP_FAILED)
    {
      ASSERT_EQ(0, munmap(orig_buf, old_size)) << "Unmap the original buffer.";
    }
  else
    {
      ASSERT_EQ(0, munmap(new_buf, new_size)) << "Unmap the new buffer.";
    }
}
