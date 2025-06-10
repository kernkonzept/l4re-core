/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include "ro_file.h"

#include <sys/ioctl.h>

#include <l4/re/env>

namespace L4Re { namespace Core {

Ro_file::~Ro_file() noexcept
{
  if (_addr)
    L4Re::Env::env()->rm()->detach(l4_addr_t(_addr), 0);

  L4Re::virt_cap_alloc->release(_ds);
}

int
Ro_file::fstat(struct stat64 *buf) const noexcept
{
  static int fake = 0;

  memset(buf, 0, sizeof(*buf));
  buf->st_size = _size;
  buf->st_mode = S_IFREG | 0644;
  buf->st_dev = _ds.cap();
  buf->st_ino = ++fake;
  buf->st_blksize = L4_PAGESIZE;
  buf->st_blocks = l4_round_page(_size);
  return 0;
}

ssize_t
Ro_file::read_single(const struct iovec *vec, off64_t pos) noexcept
{
  off64_t l = vec->iov_len;
  if (_size - pos < l)
    l = _size - pos;

  if (l > 0)
    {
      Vfs_config::memcpy(vec->iov_base, _addr + pos, l);
      return l;
    }

  return 0;
}

ssize_t
Ro_file::preadv(const struct iovec *vec, int cnt, off64_t offset) noexcept
{
  if (!_addr)
    {
      void const *file = reinterpret_cast<void*>(L4_PAGESIZE);
      long err = L4Re::Env::env()->rm()->attach(&file, _size,
                                                Rm::F::Search_addr | Rm::F::R,
                                                _ds, 0);

      if (err < 0)
        return err;

      _addr = static_cast<char const *>(file);
    }

  ssize_t l = 0;

  while (cnt > 0)
    {
      ssize_t r = read_single(vec, offset);
      offset += r;
      l += r;

      if (static_cast<size_t>(r) < vec->iov_len)
        return l;

      ++vec;
      --cnt;
    }
  return l;
}

ssize_t
Ro_file::pwritev(const struct iovec *, int, off64_t) noexcept
{
  return -EROFS;
}

int
Ro_file::ioctl(unsigned long v, va_list args) noexcept
{
  switch (v)
    {
    case FIONREAD: // return amount of data still available
      int *available = va_arg(args, int *);
      *available = _size - pos();
      return 0;
    };
  return -ENOTTY;
}

}}
