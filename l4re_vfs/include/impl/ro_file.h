/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/l4re_vfs/backend>

namespace L4Re { namespace Core {

class Ro_file : public L4Re::Vfs::Be_file_pos
{
private:
  L4::Cap<L4Re::Dataspace> _ds;
  off64_t _size;
  char const *_addr;

public:
  explicit Ro_file(L4::Cap<L4Re::Dataspace> ds) throw()
  : Be_file_pos(), _ds(ds), _addr(0)
  {
    _size = _ds->size();
  }

  L4::Cap<L4Re::Dataspace> data_space() const throw() { return _ds; }

  int fstat64(struct stat64 *buf) const throw();

  int ioctl(unsigned long, va_list) throw();

  off64_t size() const throw() { return _size; }

  int get_status_flags() const throw()
  { return O_RDONLY; }

  int set_status_flags(long) throw()
  { return 0; }

  ~Ro_file() throw();

private:
  ssize_t read_single(const struct iovec*, off64_t) throw();
  ssize_t preadv(const struct iovec *, int, off64_t) throw();
  ssize_t pwritev(const struct iovec *, int , off64_t) throw();
};


}}
