/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <sys/time.h>
#include <time.h>
#include <l4/re/env.h>

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  struct timespec ts;
  int e = clock_gettime(CLOCK_REALTIME, &ts);

  (void)tz;
  if (e < 0)
    return e;

  if (tz)
    tz->tz_minuteswest = tz->tz_dsttime = 0;

  tv->tv_sec  = ts.tv_sec;
  tv->tv_usec = ts.tv_nsec / 1000;
  return 0;
}
