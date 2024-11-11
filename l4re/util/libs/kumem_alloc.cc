/*!
 * \file
 * \brief  Kumem allocator helper
 */
/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/sys/task>
#include <l4/re/util/kumem_alloc>

namespace L4Re { namespace Util {

int
kumem_alloc(l4_addr_t *v, unsigned pages_order,
            L4::Cap<L4::Task> task,
            L4::Cap<L4Re::Rm> rm) noexcept
{
  int r;
  unsigned long sz = (1 << pages_order) * L4_PAGESIZE;
  unsigned sh = pages_order + L4_PAGESHIFT;

#ifdef CONFIG_MMU
  // On MMU systems, user space chooses the spot in the virtual address space.
  *v = 0;
  if ((r = rm->reserve_area(v, sz,
                            L4Re::Rm::F::Reserved | L4Re::Rm::F::Search_addr,
                            sh)))
    return r;

  auto fp = l4_fpage(*v, sh, L4_FPAGE_RW);
  if ((r = l4_error(task->add_ku_mem(&fp))))
    {
      rm->free_area(*v);
      return r;
    }
#else
  // On systems without MMU the kernel determines the actual location.
  auto fp = l4_fpage(0, sh, L4_FPAGE_RW);
  if ((r = l4_error(task->add_ku_mem(&fp))))
    return r;

  *v = l4_fpage_memaddr(fp);
  // There is no point in checking the result of the region manager. The kernel
  // allocated the address so it is known to be valid. And there is no facility
  // to release the allocated ku_mem anyway.
  rm->reserve_area(v, sz, L4Re::Rm::F::Reserved);
#endif

  return 0;
}

}}
