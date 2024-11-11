/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/l4re_vfs/vfs.h>

namespace L4Re { namespace Core {

using cxx::Ref_ptr;

class Fd_store
{
public:
  enum { MAX_FILES = 50 };

  Fd_store() noexcept : _fd_hint(0) {}

  int alloc() noexcept;
  void free(int fd) noexcept;
  bool check_fd(int fd) noexcept;
  Ref_ptr<L4Re::Vfs::File> get(int fd) noexcept;
  void set(int fd, Ref_ptr<L4Re::Vfs::File> const &f) noexcept;

private:
  int _fd_hint;
  Ref_ptr<L4Re::Vfs::File> _files[MAX_FILES];
};

inline
bool
Fd_store::check_fd(int fd) noexcept
{
  return fd >= 0 && fd < MAX_FILES;
}

inline
Ref_ptr<L4Re::Vfs::File>
Fd_store::get(int fd) noexcept
{
  if (check_fd(fd))
    return _files[fd];

  return Ref_ptr<>::Nil;
}

inline
void
Fd_store::set(int fd, Ref_ptr<L4Re::Vfs::File> const &f) noexcept
{
  _files[fd] = f;
}

}}
