/*
 * Simple libc-backend to satisfy write(1, x, y)
 */
/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/re/env>
#include <sys/types.h>
#include <unistd.h>

extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
  if (fd == 1 || fd == 2)
    {
      L4Re::Env::env()->log()->printn((const char *)buf, count);
      return count;
    }
  return -1;
}

extern "C" ssize_t read(int, void *, size_t)
{
  return -1;
}

extern "C" __off_t lseek(int, __off_t, int) L4_NOTHROW
{
  return -1;
}

extern "C" __off64_t lseek64(int, __off64_t, int) L4_NOTHROW
{
  return -1;
}
