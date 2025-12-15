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
    {
      errno = ENOTSUP;
      return -1;
    }

  l4_timeout_t to;

  l4_kernel_clock_t sleep_val_us = ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
  l4_msgtag_t tag;
  if (flags & TIMER_ABSTIME)
    {
      if (clock_id == CLOCK_REALTIME)
        sleep_val_us -= __libc_l4_rt_clock_offset;
      l4_rcv_timeout(l4_timeout_abs(sleep_val_us, 0), &to);
      tag = l4_ipc_receive(L4_INVALID_CAP, l4_utcb(), to);
    }
  else
    tag = l4_ipc_sleep_us(sleep_val_us);

  if (l4_ipc_error(tag, l4_utcb()) != L4_IPC_RETIMEOUT)
    {
      errno = EINTR;
      return -1;
    }

  if (rem)
    {
      // We do not know really at this point
      rem->tv_sec = 0;
      rem->tv_nsec = 0;
    }

  return 0;
}
