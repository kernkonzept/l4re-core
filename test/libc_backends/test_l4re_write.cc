/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Benjamin Lamowski <benjamin.lamowski@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * \file
 * Test Posix write functions of the L4Re libc backend using a virtual console.
 */

#include "test_helpers.h"

#include <string>

#include <l4/re/util/icu_svr>
#include <l4/re/util/vcon_svr>
#include <l4/re/error_helper>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/ipc_helper>
#include <l4/atkins/tap/main>
#include <gtest/gtest.h>

// *** helper functions ********************************************************

/**
 * Return a file descriptor to the Vcon test file that closes automatically.
 */
static AutoCloseFd
open_vcon()
{
  int fd = open("/vcon", O_RDWR);

  if (fd == -1)
    L4Re::chksys(-L4_EINVAL, "Open /vcon.");

  return AutoCloseFd(fd);
}

/**
 * Virtual console backend that returns the written content.
 *
 * Note that in the current implementation, read content is erased from the
 * backing store, so the semantics differ from regular files.
 */
class Vcon_mock : public L4Re::Util::Vcon_svr<Vcon_mock>,
                  public L4Re::Util::Icu_cap_array_svr<Vcon_mock>,
                  public L4::Epiface_t<Vcon_mock, L4::Vcon>
{
public:
  Vcon_mock() : Icu_cap_array_svr<Vcon_mock>(1, &_irq) {}

  unsigned vcon_read(char *buf, unsigned size)
  {
    if (_buffer.empty())
      return 0;

    int outsize = size < _buffer.size() ? size : _buffer.size();
    memcpy(buf, _buffer.data(), outsize);
    _buffer.erase(0, outsize);

    return outsize;
  }

  void vcon_write(const char *buf, unsigned size) { _buffer.append(buf, size); }

  bool verify_write(const char *written)
  {
    return (_buffer.compare(written) == 0);
  }

private:
  std::string _buffer;
  L4Re::Util::Icu_cap_array_svr<Vcon_mock>::Irq _irq;
};


// *** fixture *****************************************************************

/**
 * Fixture setting up a Vcon mock thread.
 */
class BeL4ReWriteFamily :
  public Atkins::Fixture::Base_server_thread,
  public ::testing::Test
{
public:
  BeL4ReWriteFamily()
  {
    register_handler(L4Re::Env::env()->get_cap<L4::Rcv_endpoint>("vcon"));
    start_loop();
  }

  ~BeL4ReWriteFamily()
  {
    // Don't unmap the external capability on destruction.
    server.registry()->unregister_obj(&_handler, false);
  }

  bool verify_write(const char *written)
  {
    return _handler.verify_write(written);
  }

private:
  Vcon_mock _handler;

  void register_handler(L4::Cap<L4::Rcv_endpoint> cap)
  {
    L4Re::chkcap(server.registry()->register_obj(&_handler, cap));
  }
};

// *** write *******************************************************************

/**
 * Content written to a console file descriptor can be read back.
 */
TEST_F(BeL4ReWriteFamily, WriteRegular)
{
  auto fd = open_vcon();
  char const *text = "boomerang";
  ssize_t const textsize = strlen(text);

  ASSERT_EQ(textsize, write(fd.get(), text, textsize))
    << "Write to Vcon file descriptor.";
  ASSERT_TRUE(this->verify_write(text))
    << "Content written to backend.";
}

/**
 * A write with 0 length does not write to the file descriptor and returns 0.
 */
TEST_F(BeL4ReWriteFamily, WriteZeroLength)
{
  auto fd = open_vcon();
  char const *text = "BOOMERANG";

  ASSERT_EQ(0, write(fd.get(), text, 0)) << "Write with zero length.";
  ASSERT_TRUE(this->verify_write(""))
    << "Buffer was not written.";
}

// *** pwrite ******************************************************************

/**
 * The pwrite() Posix function can be called with offset 0 and the written
 * content read back.
 */
TEST_F(BeL4ReWriteFamily, PWriteOffsetZero)
{
  auto fd = open_vcon();
  char const *text = "boomerang";
  ssize_t const textsize = strlen(text);

  ASSERT_EQ(textsize, pwrite(fd.get(), text, textsize, 0))
    << "Call pwrite() on the Vcon file descriptor with offset 0.";
  ASSERT_TRUE(this->verify_write(text))
    << "Content written to backend.";
}
