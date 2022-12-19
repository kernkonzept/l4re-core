/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/capability>
#include <l4/sys/vcon>
#include <l4/sys/semaphore>

#include <l4/l4re_vfs/backend>

namespace L4Re { namespace Core {

class Vcon_stream : public L4Re::Vfs::Be_file_stream
{
private:
  L4::Cap<L4::Vcon> _s;
  L4::Cap<L4::Semaphore>  _irq;
  unsigned _irq_bound;

public:
  explicit Vcon_stream(L4::Cap<L4::Vcon> s) noexcept;

  ssize_t readv(const struct iovec*, int iovcnt) noexcept override;
  ssize_t writev(const struct iovec*, int iovcnt) noexcept override;
  int fstat64(struct stat64 *buf) const noexcept override;
  int get_status_flags() const noexcept override { return O_RDWR; }
  int set_status_flags(long) noexcept override { return 0; }
  int ioctl(unsigned long request, va_list args) noexcept override;

  ~Vcon_stream() noexcept {}
  void operator delete (void *) {}
};

}}
