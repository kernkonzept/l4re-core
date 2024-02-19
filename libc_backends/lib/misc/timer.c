/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>

int timer_delete(timer_t timer_id)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  errno = -EINVAL;
  return -1;
}

int timer_gettime(timer_t timer_id, struct itimerspec *setting)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  (void)setting;
  errno = -EINVAL;
  return -1;
}

int timer_settime(timer_t timer_id, int __flags,
                  const struct itimerspec *__restrict __value,
                  struct itimerspec *__restrict __ovalue)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  (void)__value;
  (void)__ovalue;
  (void)__flags;
  errno = -EINVAL;
  return -1;
}

int timer_create (clockid_t __clock_id,
                  struct sigevent *__restrict __evp,
                  timer_t *__restrict __timerid)
{
  printf("Unimplemented: %s(clock_id)\n", __func__);
  (void)__clock_id;
  (void)__evp;
  (void)__timerid;
  errno = -EINVAL;
  return -1;
}
