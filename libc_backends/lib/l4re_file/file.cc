/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */


#include <features.h>

#ifndef  __USE_ATFILE
# define __USE_ATFILE 1
#endif

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
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <l4/l4re_vfs/backend>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "redirect.h"

using namespace L4Re::Vfs;
using cxx::Ref_ptr;

ssize_t read(int fd, void *buf, size_t count)
{
  struct iovec iov;
  iov.iov_base = buf;
  iov.iov_len = count;
  return readv(fd, &iov, 1);
}

ssize_t write(int fd, const void *buf, size_t count)
{
  struct iovec iov;
  iov.iov_base = const_cast<void*>(buf);
  iov.iov_len = count;
  return writev(fd, &iov, 1);
}

static void copy_stat64_to_stat(struct stat *buf, struct stat64 *sb64)
{
  memset(buf, 0, sizeof(*buf));

  buf->st_dev = sb64->st_dev;
  buf->st_ino = sb64->st_ino;
  buf->st_mode = sb64->st_mode;
  buf->st_nlink = sb64->st_nlink;
  buf->st_uid = sb64->st_uid;
  buf->st_gid = sb64->st_gid;
  buf->st_rdev = sb64->st_rdev;
  buf->st_size = sb64->st_size;
  buf->st_blksize = sb64->st_blksize;
  buf->st_blocks = sb64->st_blocks;
  buf->st_atime = sb64->st_atime;
  buf->st_mtime = sb64->st_mtime;
  buf->st_ctime = sb64->st_ctime;
}

int fstat(int fd, struct stat *buf) L4_NOTHROW
{
  struct stat64 sb64;
  int r = fstat64(fd, &sb64);
  if (r < 0)
    return r;

  copy_stat64_to_stat(buf, &sb64);
  return r;
}

#define ERRNO_RET(r) do { \
  if ((r) < 0) \
    {          \
      errno = -(r); \
      return -1; \
    } } while (0)


namespace {

static Ref_ptr<File> __internal_get_dir(int dirfd, char const **path) L4_NOTHROW
{
  Ops *vfs_ops = L4Re::Vfs::vfs_ops;
  if (**path == '/')
    {
      while (**path == '/')
	++*path;

      return vfs_ops->get_root();
    }
  else if (dirfd == AT_FDCWD)
    return vfs_ops->get_cwd();
  else
    return vfs_ops->get_file(dirfd);
}

static char const *
__internal_resolvedir(int dirfd, const char *path, int flags, mode_t mode,
                      Ref_ptr<File> *f) L4_NOTHROW
{
  (void)flags;
  (void)mode;
  Ref_ptr<File> dir = __internal_get_dir(dirfd, &path);
  if (!dir)
    return 0;

  return dir->get_mount(path, f);
}

static int
__internal_resolve(int dirfd, const char *path, int flags, mode_t mode,
                   Ref_ptr<File> *f) L4_NOTHROW
{
  Ref_ptr<File> dir = __internal_get_dir(dirfd, &path);
  if (!dir)
    return -EBADF;

  return dir->openat(path, flags, mode, f);
}

static int
__internal_open(const char *path, int flags, mode_t mode) L4_NOTHROW
{
  Ref_ptr<File> f;
  int res = __internal_resolve(AT_FDCWD, path, flags, mode, &f);

  ERRNO_RET(res);

  int fd = L4Re::Vfs::vfs_ops->alloc_fd(f);

  ERRNO_RET(fd);
  return fd;
}
}

int open(const char *name, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list v;
      va_start(v, flags);
      mode = va_arg(v, mode_t);
      va_end(v);
    }

  return __internal_open(name, flags, mode);

}

int open64(const char *name, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list v;
      va_start(v, flags);
      mode = va_arg(v, mode_t);
      va_end(v);
    }

  return __internal_open(name, flags, mode);
}

extern "C" int ioctl(int fd, unsigned long request, ...) L4_NOTHROW
{
  va_list v;
  va_start(v, request);

  L4B_FD;

  int r = file->ioctl(request, v);
  va_end(v);
  POST();
}

#if !(defined(__USE_LARGEFILE64) && !defined(__LP64__))
// Duplicate here as uclibc only defines fcntl64
// with __USE_LARGEFILE64 && !__LP64__
extern "C" int fcntl64 (int __fd, int __cmd, ...);
#endif

extern "C" int fcntl64(int fd, int cmd, ...)
{
  Ops *o = L4Re::Vfs::vfs_ops;
  Ref_ptr<File> f = o->get_file(fd);
  switch (cmd)
    {
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
      // 'arg' has the lowest fd ... so dup isn't the correct thing
      return dup(fd);

    case F_GETFD:
      return 0;
    case F_SETFD:
      return 0;

    case F_GETFL:
      return f->get_status_flags();
    case F_SETFL:
      return 0;

    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      errno = EINVAL;
      return -1;

    case F_GETOWN:
      return 0;
    case F_SETOWN:
      errno = EINVAL;
      return -1;

    case F_GETSIG:
      return 0;
    case F_SETSIG:
      errno = EINVAL;
      return -1;

    default:
      errno = EINVAL;
      return -1;
    }
}

extern "C" int fcntl(int fd, int cmd, ...)
{
  unsigned long arg;
  va_list v;

  va_start(v, cmd);
  arg = va_arg(v, unsigned long);
  va_end(v);

  return fcntl64(fd, cmd, arg);
}


off_t lseek(int fd, off_t offset, int whence) L4_NOTHROW
{
  return lseek64(fd, offset, whence);
}

int ftruncate(int fd, off_t length) L4_NOTHROW
{
  return ftruncate64(fd, length);
}

int lockf(int fd, int cmd, off_t len)
{
  (void)fd;
  (void)cmd;
  (void)len;
  errno = EINVAL;
  return -1;
}



extern "C" int dup2(int oldfd, int newfd) L4_NOTHROW
{
  Ops *o = L4Re::Vfs::vfs_ops;
  Ref_ptr<File> oldf = o->get_file(oldfd);
  if (!oldf)
    {
      errno = EBADF;
      return -1;
    }

  Ref_ptr<File> newf = o->set_fd(newfd, oldf);
  if (!newf || newf == oldf)
    return newfd;

  // do the stuff for close;
  newf->unlock_all_locks();

  return newfd;
}

extern "C" int dup(int oldfd) L4_NOTHROW
{
  Ops *o = L4Re::Vfs::vfs_ops;
  Ref_ptr<File> f = o->get_file(oldfd);
  if (!f)
    {
      errno = EBADF;
      return -1;
    }

  int r = o->alloc_fd(f);
  ERRNO_RET(r);
  return r;
}


int stat(const char *path, struct stat *buf) L4_NOTHROW
{
  struct stat64 sb64;
  int r = stat64(path, &sb64);
  if (r < 0)
    return r;

  copy_stat64_to_stat(buf, &sb64);
  return r;
}

int lstat(const char *path, struct stat *buf) L4_NOTHROW
{
  struct stat64 sb64;
  int r = lstat64(path, &sb64);

  if (r < 0)
    return r;

  copy_stat64_to_stat(buf, &sb64);
  return r;
}

int close(int fd) L4_NOTHROW
{
  Ops *o = L4Re::Vfs::vfs_ops;
  Ref_ptr<File> f = o->free_fd(fd);
  if (!f)
    {
      errno = EBADF;
      return -1;
    }

  f->unlock_all_locks();
  return 0;
}

int access(const char *path, int mode) L4_NOTHROW
{ return faccessat(AT_FDCWD, path, mode, 0); }

extern "C" ssize_t __getdents64(int fd, char *buf, size_t nbytes);
extern "C" ssize_t __getdents64(int fd, char *buf, size_t nbytes)
{
  Ops *o = L4Re::Vfs::vfs_ops;
  Ref_ptr<File> fdo = o->get_file(fd);
  if (!fdo)
    {
      errno = EBADF;
      return -1;
    }

  ssize_t r = fdo->getdents(buf, nbytes);
  if (r < 0)
    {
      errno = -r;
      return -1;
    }

  return r;
}

#if __WORDSIZE == 64
extern "C" ssize_t __getdents(int, char *, size_t);
L4_STRONG_ALIAS(__getdents64,__getdents)
#endif


#define L4B_REDIRECT(ret, func, ptlist, plist) \
  extern "C" ret func ptlist L4_NOTHROW                                    \
  {                                                             \
    cxx::Ref_ptr<L4Re::Vfs::File> file;                   \
    int res = __internal_resolve(AT_FDCWD, _a1, 0, 0, &file);   \
    ERRNO_RET(res);                                             \
    ret r = file->L4B_REDIRECT_FUNC(func)(L4B_STRIP_FIRST(plist));  \
    POST();                                                     \
  }

#define L4B_REDIRECT_FUNC(func) func
L4B_REDIRECT_3(ssize_t, readlink, const char *, char *, size_t)
L4B_REDIRECT_2(int,     utime,    const char *, const struct utimbuf *)
L4B_REDIRECT_2(int,     utimes,   const char *, const struct timeval *)
#undef L4B_REDIRECT_FUNC

#define L4B_REDIRECT_FUNC(func) f##func
L4B_REDIRECT_2(int,       stat64,      const char *, struct stat64 *)
L4B_REDIRECT_2(int,       chmod,       const char *, mode_t)
#undef L4B_REDIRECT_FUNC

#define L4B_REDIRECT_FUNC(func) fstat64
L4B_REDIRECT_2(int,       lstat64,     const char *, struct stat64 *)
#undef L4B_REDIRECT_FUNC
#undef L4B_REDIRECT

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  extern "C" ret func ptlist L4_NOTHROW                         \
  {                                                             \
    cxx::Ref_ptr<L4Re::Vfs::File> dir;                          \
    _a1 = __internal_resolvedir(AT_FDCWD, _a1, 0, 0, &dir);     \
    ret r = dir->func plist;     \
    POST();                                                     \
  }

L4B_REDIRECT_1(int, unlink,  const char *)
L4B_REDIRECT_2(int, mkdir,   const char *, mode_t)
L4B_REDIRECT_1(int, rmdir,   const char *)
#undef L4B_REDIRECT

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  extern "C" ret func ptlist L4_NOTHROW                         \
  {                                                             \
    cxx::Ref_ptr<L4Re::Vfs::File> dir1;                         \
    cxx::Ref_ptr<L4Re::Vfs::File> dir2;                         \
    _a1 = __internal_resolvedir(AT_FDCWD, _a1, 0, 0, &dir1);    \
    _a2 = __internal_resolvedir(AT_FDCWD, _a2, 0, 0, &dir2);    \
    ret r = dir1->func plist;     \
    POST();                                                     \
  }

L4B_REDIRECT_2(int, rename,  const char *, const char *)
L4B_REDIRECT_2(int, link,    const char *, const char *)
L4B_REDIRECT_2(int, symlink, const char *, const char *)
#undef L4B_REDIRECT

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  extern "C" ret func ptlist L4_NOTHROW                         \
  {                                                             \
    cxx::Ref_ptr<L4Re::Vfs::File> file;                         \
    int res = __internal_resolve(AT_FDCWD, _a1, 0, 0, &file);   \
    ERRNO_RET(res);                                             \
    ret r = file->ftruncate64(L4B_STRIP_FIRST(plist));  \
    POST();                                                     \
  }

L4B_REDIRECT_2(int, truncate, const char *, off_t)
L4B_REDIRECT_2(int, truncate64, const char *, off64_t)

#undef L4B_REDIRECT

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  extern "C" ret func ptlist L4_NOTHROW                         \
  {                                                             \
    cxx::Ref_ptr<L4Re::Vfs::File> dir;                          \
    _a2 = __internal_resolvedir(_a1, _a2, 0, 0, &dir);          \
    if (!_a2) \
      { \
	errno = EBADF; \
	return -1; \
      } \
    ret r = dir->func(L4B_STRIP_FIRST(plist));                  \
    POST();                                                     \
  }

//L4B_REDIRECT_3(int, unlinkat,  int, const char *, int)
L4B_REDIRECT_4(int,       faccessat,   int, const char *, int, int)


// ------------------------------------------------------

//#include <stdio.h>
#include <l4/util/util.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout)
{
  (void)nfds; (void)readfds; (void)writefds; (void)exceptfds;
  //printf("Call: %s(%d, %p, %p, %p, %p[%ld])\n", __func__, nfds, readfds, writefds, exceptfds, timeout, timeout->tv_usec + timeout->tv_sec * 1000000);

#if 0
  int us = timeout->tv_usec + timeout->tv_sec * 1000000;
  l4_timeout_t to = l4_timeout(L4_IPC_TIMEOUT_NEVER,
                               l4util_micros2l4to(us));
#endif

  // only the timeout for now
  if (timeout)
    l4_usleep(timeout->tv_usec + timeout->tv_sec * 1000000);
  else
    l4_sleep_forever();

  return 0;
}

#undef L4B_REDIRECT

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  ret func ptlist L4_NOTHROW \
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

__BEGIN_DECLS
ssize_t preadv(int, const struct iovec *, int, off_t);
ssize_t pwritev(int, const struct iovec *, int, off_t);
__END_DECLS

L4B_REDIRECT_2(int,       fstat64,     int, struct stat64 *)
L4B_REDIRECT_3(ssize_t,   readv,       int, const struct iovec *, int)
L4B_REDIRECT_3(ssize_t,   writev,      int, const struct iovec *, int)
L4B_REDIRECT_4(ssize_t,   preadv,      int, const struct iovec *, int, off_t)
L4B_REDIRECT_4(ssize_t,   pwritev,     int, const struct iovec *, int, off_t)
L4B_REDIRECT_3(__off64_t, lseek64,     int, __off64_t, int)
L4B_REDIRECT_2(int,       ftruncate64, int, off64_t)
L4B_REDIRECT_1(int,       fsync,       int)
L4B_REDIRECT_1(int,       fdatasync,   int)
L4B_REDIRECT_2(int,       fchmod,      int, mode_t)

static char const * const _default_current_working_dir = "/";
static char *_current_working_dir = const_cast<char *>(_default_current_working_dir);

static void free_cwd()
{
  if (_current_working_dir != _default_current_working_dir)
    free(_current_working_dir);
}

extern "C" int chdir(const char *path) L4_NOTHROW
{
  Ref_ptr<File> f;
  int res = __internal_resolve(AT_FDCWD, path, 0, 0, &f);
  ERRNO_RET(res);

  if (*path == '/')
    {
      free_cwd();
      _current_working_dir = strdup(path);
    }
  else
    {
      unsigned len_cwd = strlen(_current_working_dir);
      unsigned len_path = strlen(path);
      char *tmp = (char *)malloc(len_cwd + len_path + 2);
      memcpy(tmp, _current_working_dir, len_cwd);
      if (tmp[len_cwd - 1] != '/')
        tmp[len_cwd++] = '/';
      memcpy(tmp + len_cwd, path, len_path + 1);

      free_cwd();
      _current_working_dir = tmp;
    }

  // would need to check whether 'f' is a directory
  L4Re::Vfs::vfs_ops->set_cwd(f);

  return 0;
}

extern "C" int fchdir(int fd) L4_NOTHROW
{
  L4Re::Vfs::Ops *o = L4Re::Vfs::vfs_ops;
  cxx::Ref_ptr<L4Re::Vfs::File> f = o->get_file(fd);
  if (!f)
    {
      errno = EBADF;
      return -1;
    }

  // would need to check whether 'f' is a directory
  o->set_cwd(f);
  return -1;
}

extern "C"
ssize_t pread64(int fd, void *buf, size_t count, off64_t offset)
{
  struct iovec iov;
  iov.iov_base = buf;
  iov.iov_len = count;
  return preadv(fd, &iov, 1, offset);
}

extern "C"
ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
  return pread64(fd, buf, count, offset);
}

extern "C"
ssize_t pwrite64(int fd, const void *buf, size_t count, off64_t offset)
{
  struct iovec iov;
  iov.iov_base = const_cast<void*>(buf);
  iov.iov_len = count;
  return pwritev(fd, &iov, 1, offset);
}

extern "C"
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
  return pwrite64(fd, buf, count, offset);
}

extern "C" char *getcwd(char *buf, size_t size) L4_NOTHROW
{
  unsigned len_cwd = strlen(_current_working_dir) + 1;

  if (buf == 0 && size == 0)
    size = len_cwd;

  if (buf == 0)
    buf = (char *)malloc(size);

  if (buf == 0)
    {
      errno = ENOMEM;
      return 0;
    }

  if (len_cwd > size)
    {
      errno = ERANGE;
      return 0;
    }

  memcpy(buf, _current_working_dir, len_cwd);
  return buf;
}

extern "C" int chroot(const char *) L4_NOTHROW
{
  errno = EINVAL;
  return -1;
}

extern "C" int mkfifo(const char *, mode_t) L4_NOTHROW
{
  errno = EINVAL;
  return -1;
}

extern "C" int mknod(const char *, mode_t, dev_t) L4_NOTHROW
{
  errno = EINVAL;
  return -1;
}

int chown(const char *, __uid_t, __gid_t)
{
  errno = EINVAL;
  return -1;
}

int fchown(int, __uid_t, __gid_t)
{
  errno = EINVAL;
  return -1;
}


extern "C" int lchown(const char *, uid_t, gid_t) L4_NOTHROW
{
  errno = EINVAL;
  return -1;
}
