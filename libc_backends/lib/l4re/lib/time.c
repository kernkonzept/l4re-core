/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <time.h>
#include <l4/libc_backends/clk.h>

time_t time(time_t *t)
{
  struct timespec a;
  libc_backend_rt_clock_gettime(&a);
  if (t)
    *t = a.tv_sec;
  return a.tv_sec;
}
