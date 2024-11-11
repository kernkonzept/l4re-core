/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "fd_store.h"

namespace L4Re { namespace Core {

int
Fd_store::alloc() noexcept
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
Fd_store::free(int fd) noexcept
{
  _files[fd] = 0;
  if (fd < _fd_hint)
    _fd_hint = fd;
}

}}

