/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#pragma once

#define L4B(e) L4Re::Vfs::vfs_ops->e

#define POST()                           \
  if (r < 0)                             \
    {                                    \
      errno = -r;                        \
      return -1;                         \
    }                                    \
  return r


#define L4B_FD \
  cxx::Ref_ptr<L4Re::Vfs::File> file = L4Re::Vfs::vfs_ops->get_file(fd); \
  if (!file)         \
    {                \
      errno = EBADF; \
      return -1;     \
    }


#define L4B_STRIP_FIRST__(fd, p...) p
#define L4B_STRIP_FIRST(x) L4B_STRIP_FIRST__ x

#define L4B_REDIRECT_0(ret, func) L4B_REDIRECT(ret, func, (void), ())
#define L4B_REDIRECT_1(ret, func, a1) L4B_REDIRECT(ret, func, (a1 _a1), (_a1))
#define L4B_REDIRECT_2(ret, func, a1, a2) L4B_REDIRECT(ret, func, (a1 _a1, a2 _a2), (_a1, _a2))
#define L4B_REDIRECT_3(ret, func, a1, a2, a3) L4B_REDIRECT(ret, func, (a1 _a1, a2 _a2, a3 _a3), (_a1, _a2, _a3))
#define L4B_REDIRECT_4(ret, func, a1, a2, a3, a4) L4B_REDIRECT(ret, func, (a1 _a1, a2 _a2, a3 _a3, a4 _a4), (_a1, _a2, _a3, _a4))
#define L4B_REDIRECT_5(ret, func, a1, a2, a3, a4, a5) L4B_REDIRECT(ret, func, (a1 _a1, a2 _a2, a3 _a3, a4 _a4, a5 _a5), (_a1, _a2, _a3, _a4, _a5))
#define L4B_REDIRECT_6(ret, func, a1, a2, a3, a4, a5, a6) L4B_REDIRECT(ret, func, (a1 _a1, a2 _a2, a3 _a3, a4 _a4, a5 _a5, a6 _a6), (_a1, _a2, _a3, _a4, _a5, _a6))

