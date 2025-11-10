/*
 * Simple libc-backend to satisfy write(1, x, y)
 */
/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/env>
#include <l4/sys/compiler.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
  if (fd == 1 || fd == 2)
    {
      L4Re::Env::env()->log()->printn(static_cast<const char *>(buf), count);
      return count;
    }

  errno = EBADF;
  return -1;
}

extern "C" ssize_t read(int, void *, size_t)
{
  errno = EBADF;
  return -1;
}

extern "C" off_t lseek(int, off_t, int) L4_NOTHROW
{
  errno = EBADF;
  return -1;
}

L4_STRONG_ALIAS(lseek, lseek64)
