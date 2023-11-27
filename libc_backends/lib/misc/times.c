/*
 * Copyright (C) 2009, 2013 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/env.h>
#include <sys/times.h>

clock_t times(struct tms *buf)
{
  // some arbitrary values
  buf->tms_utime = (clock_t)l4_kip_clock(l4re_kip());
  buf->tms_stime = 10;
  buf->tms_cutime = 0;
  buf->tms_cstime = 0;
  return (clock_t)l4_kip_clock(l4re_kip());
}
