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
#include "fd_store.h"

namespace L4Re { namespace Core {

int
Fd_store::alloc() throw()
{
  for (int i = _fd_hint; i < MAX_FILES; ++i)
    {
      if (!_files[i])
	{
	  _fd_hint = i + 1;
	  return i;
	}
    }

  return -1;
}

void
Fd_store::free(int fd) throw()
{
  _files[fd] = 0;
  if (fd < _fd_hint)
    _fd_hint = fd;
}

}}

