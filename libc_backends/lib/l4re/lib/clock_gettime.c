/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <l4/re/env.h>
#include <l4/sys/kip.h>
#include <l4/libc_backends/clk.h>

#include "clocks.h"

typedef int Get_clock(struct timespec *);
uint64_t __attribute__((weak)) __libc_l4_rt_clock_offset;

int __attribute__((weak))
libc_backend_rt_clock_gettime(struct timespec *tp)
{
  uint64_t clock;

  clock = l4_kip_clock(l4re_kip());
  clock += __libc_l4_rt_clock_offset;

  tp->tv_sec  = clock / 1000000;
  tp->tv_nsec = (clock % 1000000) * 1000;

  return 0;
}

static int mono_clock_gettime(struct timespec *tp)
{
  uint64_t clock;
  clock = l4_kip_clock(l4re_kip());
  tp->tv_sec = clock / 1000000;
  tp->tv_nsec = (clock % 1000000) * 1000;

  return 0;
}

Get_clock *__libc_l4_gettime[4] =
{
  [CLOCK_REALTIME]  = libc_backend_rt_clock_gettime,
  [CLOCK_MONOTONIC] = mono_clock_gettime
};

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
  if (clk_id >= NCLOCKS || !__libc_l4_gettime[clk_id])
    {
      errno = ENODEV;
      return -1;
    }

  return __libc_l4_gettime[clk_id](tp);
}

