/**
 * \file   dietlibc/lib/backends/simple_sleep/sleep.c
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>

#include <l4/util/util.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
  // l4_timeout_from_us allows a maximum timeout of 610d 14m 35s.
  l4_uint64_t us;

  (void)rem;
  if (req == NULL)
    {
      errno = EFAULT; // or maybe EINVAL ???
      return -1;
    }

  if (req->tv_nsec < 0 || req->tv_nsec > 999999999 || req->tv_sec < 0)
    {
      errno = EINVAL;
      return -1;
    }

  // __time_t could be 32-bit as well as 64-bit!
  if ((l4_uint64_t)req->tv_sec > (~0ULL / 1000000) - 1)
    us = L4_TIMEOUT_US_MAX;
  else
    us = ((l4_uint64_t)req->tv_sec * 1000000) + (req->tv_nsec / 1000);
  l4_usleep(us);

  return 0;
}
