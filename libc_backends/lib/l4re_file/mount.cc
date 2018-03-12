/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */


#include <sys/mount.h>
#include <l4/l4re_vfs/backend>

int mount(__const char *__special_file, __const char *__dir,
	  __const char *__fstype, unsigned long int __rwflag,
	  __const void *__data) __THROW
{
  int e = L4Re::Vfs::vfs_ops->mount(__special_file, __dir, __fstype, __rwflag, __data);
  if (e < 0)
    {
      errno = -e;
      return -1;
    }
  return 0;
}
