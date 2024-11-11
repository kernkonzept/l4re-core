/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/backtrace.h>

int
l4util_backtrace(void **pc_array, int max)
{
  (void)pc_array;
  (void)max;
  return 0;
}
