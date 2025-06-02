/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <pty.h>

int tcsendbreak(int fd, int duration)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)fd; (void)duration;
  errno = EINVAL;
  return -1;
}

void cfmakeraw(struct termios *termios_p)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)termios_p;
}

int openpty(int *amaster, int *aslave, char *name,
            const struct termios *termp, const struct winsize *winp)
{
  printf("Unimplemented: %s(.., .., %s, ..)\n", __func__, name);
  (void)amaster; (void)aslave;
  (void)termp; (void)winp;
  errno = EINVAL;
  return -1;
}
