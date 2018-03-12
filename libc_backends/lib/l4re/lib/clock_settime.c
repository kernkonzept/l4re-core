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

#include "clocks.h"

typedef int Get_clock(const struct timespec *);
extern uint64_t __libc_l4_rt_clock_offset;

static int rt_clock_settime(const struct timespec *tp)
{
  uint64_t clock;

  clock = tp->tv_sec * 1000000 + tp->tv_nsec / 1000;
  __libc_l4_rt_clock_offset = clock - l4_kip_clock(l4re_kip());
  return 0;
}

Get_clock *__libc_l4_settime[4] =
{
  [CLOCK_REALTIME]  = rt_clock_settime,
};

int clock_settime(clockid_t clk_id, const struct timespec *tp)
{
  if (clk_id >= NCLOCKS || !__libc_l4_settime[clk_id])
    {
      errno = ENODEV;
      return -1;
    }

  return __libc_l4_settime[clk_id](tp);
}

