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

#include <l4/l4re_vfs/vfs.h>

namespace L4Re { namespace Core {

using cxx::Ref_ptr;

class Fd_store
{
public:
  enum { MAX_FILES = 50 };

  Fd_store() throw() : _fd_hint(0) {}

  int alloc() throw();
  void free(int fd) throw();
  Ref_ptr<L4Re::Vfs::File> get(int fd) throw();
  void set(int fd, Ref_ptr<L4Re::Vfs::File> const &f) throw();

private:
  int _fd_hint;
  Ref_ptr<L4Re::Vfs::File> _files[MAX_FILES];
};


inline
Ref_ptr<L4Re::Vfs::File>
Fd_store::get(int fd) throw()
{
  if (fd >= 0 && fd < MAX_FILES)
    return _files[fd];

  return Ref_ptr<>::Nil;
}

inline
void
Fd_store::set(int fd, Ref_ptr<L4Re::Vfs::File> const &f) throw()
{
  _files[fd] = f;
}

}}
