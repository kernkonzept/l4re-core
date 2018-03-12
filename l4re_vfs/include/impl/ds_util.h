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
#include <l4/re/dataspace>
#include <l4/re/env>
// #include <l4/sys/kdebug.h>

namespace L4Re { namespace Core {

static
bool
release_ds(L4::Cap<L4Re::Dataspace> const &c)
{
  int l = c.validate(L4Re::This_task).label();
  // outstring("RELEASE CAP: "); outhex32(c.cap()); outstring(" ");
  // outhex32(l); outstring("\n");
  //printf("release_cap: %lx %d\n", c.cap(), l);
  if (!l)
    {
      Vfs_config::cap_alloc.free(c);
      return true;
    }

  return false;
}


}}
