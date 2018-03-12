/*!
 * \file
 * \brief  Kumem allocator helper
 */
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

#include <l4/sys/task>
#include <l4/re/util/kumem_alloc>

namespace L4Re { namespace Util {

int
kumem_alloc(l4_addr_t *v, unsigned pages_order,
            L4::Cap<L4::Task> task,
            L4::Cap<L4Re::Rm> rm) throw()
{
  int r;
  unsigned long sz = (1 << pages_order) * L4_PAGESIZE;
  unsigned sh = pages_order + L4_PAGESHIFT;

  *v = 0;
  if ((r = rm->reserve_area(v, sz,
                            L4Re::Rm::Reserved | L4Re::Rm::Search_addr,
                            sh)))
    return r;

  if ((r = l4_error(task->add_ku_mem(l4_fpage(*v, sh, L4_FPAGE_RW)))))
    {
      rm->free_area(*v);
      return r;
    }

  return 0;
}

}}
