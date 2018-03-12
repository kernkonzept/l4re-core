/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
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

int statfs(const char *path, struct statfs *buf)
{
  printf("%s(%s, %p): unimplemented\n", __func__, path, buf);
  errno = ENOENT;
  return -1;
}

int fstatfs(int fd, struct statfs *buf)
{
  printf("%s(%d, %p): unimplemented\n", __func__, fd, buf);
  errno = ENOENT;
  return -1;
}

