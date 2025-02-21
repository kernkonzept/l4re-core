/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */


#include <sys/mount.h>
#include <l4/l4re_vfs/backend>

int mount(const char *__special_file, const char *__dir,
	  const char *__fstype, unsigned long int __rwflag,
	  const void *__data)
noexcept(noexcept(mount(__special_file, __dir, __fstype, __rwflag, __data)))
{
  int e = L4Re::Vfs::vfs_ops->mount(__special_file, __dir, __fstype, __rwflag, __data);
  if (e < 0)
    {
      errno = -e;
      return -1;
    }
  return 0;
}
