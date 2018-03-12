/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/*
 * Some convenience functions for memory descriptors.
 */

#include <l4/sigma0/kip.h>
#include <l4/sys/memdesc.h>
#include <l4/util/memdesc.h>

l4_addr_t
l4util_memdesc_vm_high(l4_kernel_info_t *kinfo)
{
  l4_kernel_info_mem_desc_t *md = l4_kernel_info_get_mem_descs(kinfo);
  int nr = l4_kernel_info_get_num_mem_descs(kinfo);
  int i;

  for (i = 0; i < nr; i++, md++)
    if (l4_kernel_info_get_mem_desc_is_virtual(md))
      return l4_kernel_info_get_mem_desc_end(md);

  return 0;
}
