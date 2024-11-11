/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
