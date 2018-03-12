#include <features.h>

#include <l4/util/atomic.h>
#include <l4/re/log>
#include <l4/re/env>

#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <l4/l4re_vfs/backend>
#include <string.h>
#include <stdlib.h>
#include "redirect.h"

using namespace L4Re::Vfs;
using cxx::Ref_ptr;


#define ERRNO_RET(r) do { \
  if ((r) < 0) \
    {          \
      errno = -(r); \
      return -1; \
    } } while (0)

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  ret func ptlist __THROW \
  {               \
    L4Re::Vfs::Ops *o = L4Re::Vfs::vfs_ops; \
    cxx::Ref_ptr<L4Re::Vfs::File> f = o->get_file(_a1); \
    if (!f) \
      { \
	errno = EBADF; \
	return -1; \
      } \
    ret r = f->func(L4B_STRIP_FIRST(plist)); \
    POST(); \
  }

L4B_REDIRECT_3(int, bind,    int, sockaddr const *, socklen_t)
L4B_REDIRECT_3(int, connect, int, sockaddr const *, socklen_t)

L4B_REDIRECT_4(ssize_t, send,     int, void const *, size_t, int)
L4B_REDIRECT_4(ssize_t, recv,     int, void *,       size_t, int)
L4B_REDIRECT_6(ssize_t, sendto,   int, void const *, size_t, int, sockaddr const *, socklen_t)
L4B_REDIRECT_6(ssize_t, recvfrom, int, void *,       size_t, int, sockaddr *, socklen_t *)

L4B_REDIRECT_3(ssize_t, sendmsg,  int, msghdr const *, int)
L4B_REDIRECT_3(ssize_t, recvmsg,  int, msghdr *, int)

L4B_REDIRECT_5(int, getsockopt, int, int, int, void *,       socklen_t *)
L4B_REDIRECT_5(int, setsockopt, int, int, int, void const *, socklen_t)
L4B_REDIRECT_2(int, listen, int, int)
L4B_REDIRECT_3(int, accept, int, sockaddr *, socklen_t *)
L4B_REDIRECT_2(int, shutdown, int, int)
L4B_REDIRECT_3(int, getsockname, int, sockaddr *, socklen_t *)
L4B_REDIRECT_3(int, getpeername, int, sockaddr *, socklen_t *)


