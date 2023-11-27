/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <sched.h>
#include <errno.h>
#include <unistd.h>

int sched_get_priority_max(int policy)
{
  (void)policy;
  return 255;
}

int sched_get_priority_min(int policy)
{
  (void)policy;
  return 1;
}

int sched_getcpu(void)
{
  // In a general sense we don't know, so just report the first CPU
  return 0;
}

int sched_getaffinity(pid_t pid, size_t cpusetsize,
                      cpu_set_t *mask)
{
  (void)pid;
  __CPU_SET_S(1, cpusetsize, mask);
  return 0;
}

int nice(int inc)
{
  (void)inc;
  errno = EPERM;
  return -1;
}
