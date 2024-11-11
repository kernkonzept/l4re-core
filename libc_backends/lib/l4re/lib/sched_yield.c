/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/sys/thread.h>

int sched_yield(void);

int sched_yield(void)
{
  l4_thread_yield();
  return 0;
}
