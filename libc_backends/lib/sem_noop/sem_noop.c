/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <stdio.h>
#include <errno.h>

union semun
{
     int val;
     struct semid_ds *buf;
     unsigned short  *array;
     struct seminfo  *info;
};

key_t ftok(const char *pathname, int proj_id)
{
  printf("%s(%s, %d)\n", __func__, pathname, proj_id);
  errno = ENOSYS;
  return -1;
}

int semget(key_t key, int nsems, int semflg)
{
  printf("%s(%d, %d, %d)\n", __func__, key, nsems, semflg);
  errno = ENOSYS;
  return -1;
}

int semctl(int semid, int semnum, int cmd, ...)
{
  printf("%s(%d, %d, %d)\n", __func__, semid, semnum, cmd);
  errno = ENOSYS;
  return -1;
}

int semop(int semid, struct sembuf *sops, size_t nsops)
{
  printf("%s(%d, %p, %zd)\n", __func__, semid, sops, nsops);
  errno = ENOSYS;
  return -1;
}

