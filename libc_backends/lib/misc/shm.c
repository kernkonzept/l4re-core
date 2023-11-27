/*
 * Copyright (C) 2010 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg)
{
  printf("%s(%d, %zd, %d)\n", __func__, key, size, shmflg);
  errno = ENOSYS;
  return -1;
}

void *shmat(int shmid, const void *shmaddr, int shmflg)
{
  printf("%s(%d, %p, %d)\n", __func__, shmid, shmaddr, shmflg);
  errno = ENOSYS;
  return (void *)-1;
}

int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
  printf("%s(%d, %d, %p)\n", __func__, shmid, cmd, buf);
  errno = ENOSYS;
  return -1;
}

int shmdt(const void *shmaddr)
{
  printf("%s(%p)\n", __func__, shmaddr);
  errno = ENOSYS;
  return -1;
}
