/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int pipe(int pipefd[2])
{
  printf("Unimplemented: %s()\n", __func__);
  printf("    Caller %p\n", __builtin_return_address(0));
  (void)pipefd;
  errno = EINVAL;
  return -1;
}

FILE *popen(const char *command, const char *type)
{
  printf("Unimplemented: %s(%s, %s)\n", __func__, command, type);
  return NULL;
}

int pclose(FILE *stream)
{
  printf("Unimplemented: %s(..)\n", __func__);
  (void)stream;
  return 0;
}
