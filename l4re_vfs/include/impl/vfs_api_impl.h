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
#include "vfs_api.h"
#include <stddef.h>

namespace L4Re { namespace Core {

namespace {
class Default_cap_alloc : public L4Re::Cap_alloc
{
public:
  virtual L4::Cap<void> alloc() throw()
  { return Vfs_config::cap_alloc.alloc<void>(); }

  virtual void take(L4::Cap<void>) throw() {}

  virtual void free(L4::Cap<void> cap, l4_cap_idx_t task = L4_INVALID_CAP,
                    unsigned unmap_flags = L4_FP_ALL_SPACES) throw()
  { Vfs_config::cap_alloc.free(cap); }

  virtual bool release(L4::Cap<void> cap, l4_cap_idx_t = L4_INVALID_CAP,
                       unsigned = L4_FP_ALL_SPACES) throw()
  { return false; }

  virtual ~Default_cap_alloc() {}

  void operator delete (void *) throw() {}
};

}

L4Re::Cap_alloc *cap_alloc()
{
  static Default_cap_alloc default_cap_alloc;
  return &default_cap_alloc;
}

}}
