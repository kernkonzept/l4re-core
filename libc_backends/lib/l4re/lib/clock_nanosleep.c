/*
 * (c) 2025 Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <l4/sys/kip.h>
#include <l4/sys/types.h>
#include <l4/sys/utcb.h>
#include <l4/sys/ipc.h>
#include <l4/libc_backends/clk.h>

int clock_nanosleep(clockid_t clock_id, int flags,
                    const struct timespec *ts,
                    struct timespec *rem)
{
  switch (clock_id)
    {
    case CLOCK_REALTIME:
    case CLOCK_REALTIME_COARSE:
    case CLOCK_MONOTONIC:
    case CLOCK_MONOTONIC_RAW:
    case CLOCK_MONOTONIC_COARSE:
      break;
    default:
      return ENOTSUP;
    }

  if (!ts)
    return EFAULT;

  if (ts->tv_sec < 0 || ts->tv_nsec < 0 || ts->tv_nsec > 999999999)
    return EINVAL;

  if (rem)
    {
      rem->tv_sec = 0;
      rem->tv_nsec = 0;
    }

  l4_uint64_t sec = ts->tv_sec;
  l4_uint64_t usec = ts->tv_nsec / 1000;
  l4_kernel_clock_t abs_time_us;
  if (sec > (~0ULL - usec) / 1000000)
    abs_time_us = ~0ULL;
  else
    abs_time_us = sec * 1000000 + usec;
  if (flags == TIMER_ABSTIME)
    {
      if (clock_id == CLOCK_REALTIME || clock_id == CLOCK_REALTIME_COARSE)
        {
          if (abs_time_us < __libc_l4_rt_clock_offset)
            abs_time_us = 0;
          else
            abs_time_us -= __libc_l4_rt_clock_offset;
        }
    }
  else
    {
      l4_kernel_clock_t now = l4_kip_clock(l4_kip());
      abs_time_us = abs_time_us > ~0ULL - now ? ~0ULL : abs_time_us + now;
    }

  l4_timeout_t to;
  l4_rcv_timeout(l4_timeout_abs(abs_time_us, 0), &to);
  l4_msgtag_t tag = l4_ipc_receive(L4_INVALID_CAP, l4_utcb(), to);
  if (l4_ipc_error(tag, l4_utcb()) != L4_IPC_RETIMEOUT)
    {
      l4_kernel_clock_t now = l4_kip_clock(l4_kip());
      if (   rem
          && now < abs_time_us
          && flags != TIMER_ABSTIME) // only for relative timeout
        {
          l4_kernel_clock_t remaining = abs_time_us - now;
          rem->tv_sec = remaining / 1000000;
          rem->tv_nsec = (remaining % 1000000) * 1000;
        }
      return EINTR;
    }

  return 0;
}
