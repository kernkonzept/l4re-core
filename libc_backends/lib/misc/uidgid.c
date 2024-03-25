/*
 * Copyright (C) 2009, 2010 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <grp.h>

uid_t getuid(void) { return 123; }
uid_t getgid(void) { return 123; }
uid_t geteuid(void) { return 123; }
uid_t getegid(void) { return 123; }
pid_t getpgrp(void) { return 7; }
pid_t getppid(void) { return 122; }

int setpgid(pid_t pid, pid_t pgid)
{
  printf("Unimplemented: %s(%d, %d)\n", __func__, pid, pgid);
  errno = EPERM;
  return -1;
}


int getgroups(int size, gid_t list[])
{
  (void)size; (void)list;
  printf("Unimplemented: %s()\n", __func__);
  errno = EPERM;
  return -1;
}

int setuid(uid_t uid)
{
  printf("Unimplemented: %s(%d)\n", __func__, uid);
  return -1;
}

pid_t setsid(void)
{
  printf("Unimplemented: %s()\n", __func__);
  return -1;
}

int setgid(gid_t gid)
{
  printf("Unimplemented: %s(%d)\n", __func__, gid);
  return -1;
}

int setgroups(size_t size, const gid_t *list)
{
  (void)size;
  (void)list;
  printf("Unimplemented: %s\n", __func__);
  errno = EPERM;
  return -1;
}

int setreuid(uid_t ruid, uid_t euid)
{
  (void)ruid; (void)euid;
  printf("Unimplemented: %s\n", __func__);
  errno = EPERM;
  return -1;
}

int setregid(gid_t rgid, gid_t egid)
{
  (void)rgid; (void)egid;
  printf("Unimplemented: %s\n", __func__);
  errno = EPERM;
  return -1;
}

