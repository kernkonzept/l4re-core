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
  if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC)
    return ENOTSUP;

  l4_kernel_clock_t abs_time_us = ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
  if (flags == TIMER_ABSTIME)
    {
      if (clock_id == CLOCK_REALTIME)
        abs_time_us -= __libc_l4_rt_clock_offset;
    }
  else
    abs_time_us += l4_kip_clock(l4_kip());

  l4_timeout_t to;
  l4_rcv_timeout(l4_timeout_abs(abs_time_us, 0), &to);
  l4_msgtag_t tag = l4_ipc_receive(L4_INVALID_CAP, l4_utcb(), to);
  if (l4_ipc_error(tag, l4_utcb()) != L4_IPC_RETIMEOUT)
    return EINTR;

  if (rem)
    {
      // We do not know really at this point
      rem->tv_sec = 0;
      rem->tv_nsec = 0;
    }

  return 0;
}
