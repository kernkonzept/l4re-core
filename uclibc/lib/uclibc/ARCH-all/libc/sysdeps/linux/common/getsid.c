/* vi: set sw=4 ts=4: */
/*
 * getsid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>

#ifdef __USE_XOPEN_EXTENDED

pid_t getsid(pid_t pid)
{
  return 1;
}
libc_hidden_def(getsid)
#endif
