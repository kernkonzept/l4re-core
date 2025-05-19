/*
 * Copyright (C) 2013 TU Dresden.
 * Author(s): Björn DÖbel <doebel@os.inf.tu-dresden.de>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <sys/resource.h>

#ifdef CONFIG_L4_LIBC_UCLIBC
// uclibc-ng and glibc use a more type-safe but non-standard type for the who
// paramter
typedef __rusage_who_t __rusage_type;
#else
typedef int __rusage_type;
#endif

int getrusage(__rusage_type who, struct rusage* usage)
{
  (void)who; (void)usage;
  errno = EINVAL;
  return -1;
}
