/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Tests that the libc backend no-op implementation can be called and linked
 * from its associated library.
 */

#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <l4/atkins/tap/main>

/**
 * 'ftok()' can be called.
 */
TEST(SemNoop, FtOk)
{
  char const *path = "/dummy/path";
  int proj_id = 1;

  errno = 0;
  EXPECT_EQ(-1, ftok(path, proj_id)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'semget()' can be called.
 */
TEST(SemNoop, SemGet)
{
  key_t key = 0;
  int nsems = 1;
  int semflg = 2;

  errno = 0;
  EXPECT_EQ(-1, semget(key, nsems, semflg)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'semctl()' can be called.
 */
TEST(SemNoop, SemCtl)
{
  int semid = 0;
  int semnum = 1;
  int cmd = 2;

  errno = 0;
  EXPECT_EQ(-1, semctl(semid, semnum, cmd)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'semop()' can be called.
 */
TEST(SemNoop, SemOp)
{
  int semid = 0;
  struct sembuf sops;
  size_t nsops = 1;

  errno = 0;
  EXPECT_EQ(-1, semop(semid, &sops, nsops)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}
