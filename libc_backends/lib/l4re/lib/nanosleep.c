/**
 * \file
 * \brief  nanosleep implementation.
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>

#include <l4/sys/kip.h>
#include <l4/util/util.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
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

  l4_kernel_clock_t abs_time_us =
    l4_kip_clock(l4_kip()) + req->tv_sec * 1000000 + req->tv_nsec / 1000;

  l4_timeout_t to;
  l4_rcv_timeout(l4_timeout_abs(abs_time_us, 0), &to);
  l4_msgtag_t tag = l4_ipc_receive(L4_INVALID_CAP, l4_utcb(), to);
  (void)tag;

  return 0;
}
