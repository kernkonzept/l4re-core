/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int execv(const char *path, char *const argv[])
{
  printf("Unimplemented: %s(%s)\n", __func__, path);
  (void)argv;
  errno = EINVAL;
  return -1;
}

int execvp(const char *file, char *const argv[])
{
  printf("Unimplemented: %s(%s)\n", __func__, file);
  (void)argv;
  errno = EINVAL;
  return -1;
}

int execve(const char *filename, char *const argv[],
           char *const envp[])
{
  printf("Unimplemented: %s(%s)\n", __func__, filename);
  (void)argv;
  (void)envp;
  errno = EINVAL;
  return -1;
}

int execl(const char *path, const char *arg, ...)
{
  printf("Unimplemented: %s(%s)\n", __func__, path);
  (void)arg;
  errno = EINVAL;
  return -1;
}

int execlp(const char *file, const char *arg, ...)
{
  printf("Unimplemented: %s(%s)\n", __func__, file);
  (void)arg;
  errno = EINVAL;
  return -1;
}
