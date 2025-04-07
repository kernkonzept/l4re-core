/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>
#include <cancel.h>

/* Pause execution for a number of nanoseconds.  */
int
_NC(nanosleep) (const struct timespec *requested_time,
             struct timespec *remaining)
{
    int __ret = clock_nanosleep(CLOCK_REALTIME, 0, requested_time, remaining);

    if (__ret != 0) {
      __set_errno(__ret);
      return -1;
    }

    return __ret;
}

CANCELLABLE_SYSCALL(int, nanosleep, (const struct timespec *requested_time,
			struct timespec *remaining), (requested_time, remaining))

lt_libc_hidden(nanosleep)
