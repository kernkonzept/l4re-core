/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>


char *tmpnam(char *s);
char *tmpnam(char *s)
{
  (void)s;
  printf("%s: unimplemented\n", __func__);
  return 0;
}

#include <sys/vfs.h>

int fstatfs(int fd, struct statfs *buf)
{
  printf("%s(%d, %p): unimplemented\n", __func__, fd, buf);
  errno = ENOENT;
  return -1;
}

int fstatfs64(int fd, struct statfs64 *buf)
{
  printf("%s(%d, %p): unimplemented\n", __func__, fd, buf);
  errno = ENOENT;
  return -1;
}

