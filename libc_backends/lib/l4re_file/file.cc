/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */


#include <features.h>

#ifndef  __USE_ATFILE
# define __USE_ATFILE 1
#endif

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
#include <sys/statfs.h>
#include <sys/statvfs.h>
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

int fstat(int fd, struct stat *buf) noexcept(noexcept(fstat(fd, buf)))
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
__internal_resolvedir(int dirfd, const char *path, int /* flags */,
                      mode_t /* mode */, Ref_ptr<File> *f) L4_NOTHROW
{
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
__internal_open(int dirfd, const char *path, int flags, mode_t mode) L4_NOTHROW
{
  Ref_ptr<File> f;
  int res = __internal_resolve(dirfd, path, flags, mode, &f);

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

  return __internal_open(AT_FDCWD, name, flags, mode);
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

  return __internal_open(AT_FDCWD, name, flags, mode);
}

int openat(int dirfd, const char *name, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list v;
      va_start(v, flags);
      mode = va_arg(v, mode_t);
      va_end(v);
    }

  return __internal_open(dirfd, name, flags, mode);
}

int openat64(int dirfd, const char *name, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list v;
      va_start(v, flags);
      mode = va_arg(v, mode_t);
      va_end(v);
    }

  return __internal_open(dirfd, name, flags, mode);
}

extern "C" int ioctl(int fd, unsigned long request, ...)
noexcept(noexcept(ioctl(fd, request)))
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

  if (!f)
    {
      errno = EBADF;
      return -1;
    }

  unsigned long arg;
  va_list v;

  va_start(v, cmd);
  arg = va_arg(v, unsigned long);
  va_end(v);

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
      return f->set_status_flags(arg);

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


off_t lseek(int fd, off_t offset, int whence)
noexcept(noexcept(lseek(fd, offset, whence)))
{
  return lseek64(fd, offset, whence);
}

int ftruncate(int fd, off_t length)
noexcept(noexcept(ftruncate(fd, length)))
{
  return ftruncate64(fd, length);
}

int lockf(int /* fd */, int /* cmd */, off_t /* len */)
{
  errno = EINVAL;
  return -1;
}



extern "C" int dup2(int oldfd, int newfd)
noexcept(noexcept(dup2(oldfd, newfd)))
{
  if (newfd < 0)
    {
      errno = EBADF;
      return -1;
    }

  Ops *o = L4Re::Vfs::vfs_ops;
  Ref_ptr<File> oldf = o->get_file(oldfd);
  if (!oldf)
    {
      errno = EBADF;
      return -1;
    }

  cxx::Pair<Ref_ptr<File>, int> res = o->set_fd(newfd, oldf);
  if (res.second)
    {
      errno = res.second;
      return -1;
    }

  if (!res.first || res.first == oldf)
    return newfd;

  // do the stuff for close;
  res.first->unlock_all_locks();

  return newfd;
}

extern "C" int dup(int oldfd)
noexcept(noexcept(dup(oldfd)))
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


int stat(const char *path, struct stat *buf)
noexcept(noexcept(stat(path, buf)))
{
  struct stat64 sb64;
  int r = stat64(path, &sb64);
  if (r < 0)
    return r;

  copy_stat64_to_stat(buf, &sb64);
  return r;
}

int lstat(const char *path, struct stat *buf)
noexcept(noexcept(lstat(path, buf)))
{
  struct stat64 sb64;
  int r = lstat64(path, &sb64);

  if (r < 0)
    return r;

  copy_stat64_to_stat(buf, &sb64);
  return r;
}

int statfs([[maybe_unused]] const char *path, struct statfs *buf)
noexcept(noexcept(statfs(path, buf)))
{
  // Just stubbed for now, to be continued
  printf("l4re-statfs(%s, ...): to be implemented\n", path);
  buf->f_type = 0x6552344c;
  buf->f_bsize = L4_PAGESIZE;
  buf->f_blocks = 2;
  buf->f_bfree = 0;
  buf->f_bavail = 0;
  buf->f_files = 2;
  buf->f_ffree = 0;
  buf->f_fsid.__val[0] = 0;
  buf->f_fsid.__val[1] = 0;
  buf->f_namelen = 16;
  buf->f_frsize = 16;
  buf->f_flags = 0;
  return 0;
}

int statfs64([[maybe_unused]] const char *path, struct statfs64 *buf)
noexcept(noexcept(statfs64(path, buf)))
{
  // Just stubbed for now, to be continued
  printf("l4re-statfs64(%s, ...): to be implemented\n", path);
  buf->f_type = 0x6552344c;
  buf->f_bsize = L4_PAGESIZE;
  buf->f_blocks = 2;
  buf->f_bfree = 0;
  buf->f_bavail = 0;
  buf->f_files = 2;
  buf->f_ffree = 0;
  buf->f_fsid.__val[0] = 0;
  buf->f_fsid.__val[1] = 0;
  buf->f_namelen = 16;
  buf->f_frsize = 16;
  buf->f_flags = 0;
  return 0;
}

int statvfs([[maybe_unused]] const char *path, struct statvfs *buf) noexcept
{
  printf("l4re-statvfs(%s, ...): to be implemented\n", path);
  buf->f_bsize = L4_PAGESIZE;
  buf->f_frsize = 16;
  buf->f_blocks = 2;
  buf->f_bfree = 0;
  buf->f_bavail = 0;
  buf->f_files = 2;
  buf->f_ffree = 0;
  buf->f_favail = 0;
  buf->f_fsid = 0;
  buf->f_flag= 0;
  buf->f_namemax = 16;
  return 0;
}

int statvfs64([[maybe_unused]] const char *path, struct statvfs64 *buf) noexcept
{
  printf("l4re-statvfs64(%s, ...): to be implemented\n", path);
  buf->f_bsize = L4_PAGESIZE;
  buf->f_frsize = 16;
  buf->f_blocks = 2;
  buf->f_bfree = 0;
  buf->f_bavail = 0;
  buf->f_files = 2;
  buf->f_ffree = 0;
  buf->f_favail = 0;
  buf->f_fsid = 0;
  buf->f_flag= 0;
  buf->f_namemax = 16;
  return 0;
}

int fstatvfs([[maybe_unused]] int fd, struct statvfs *buf) noexcept
{
  printf("l4re-fstatvfs(%d, ...): to be implemented\n", fd);
  buf->f_bsize = L4_PAGESIZE;
  buf->f_frsize = 16;
  buf->f_blocks = 2;
  buf->f_bfree = 0;
  buf->f_bavail = 0;
  buf->f_files = 2;
  buf->f_ffree = 0;
  buf->f_favail = 0;
  buf->f_fsid = 0;
  buf->f_flag= 0;
  buf->f_namemax = 16;
  return 0;
}

int fstatvfs64([[maybe_unused]] int fd, struct statvfs64 *buf) noexcept
{
  printf("l4re-fstatvfs(%d, ...): to be implemented\n", fd);
  buf->f_bsize = L4_PAGESIZE;
  buf->f_frsize = 16;
  buf->f_blocks = 2;
  buf->f_bfree = 0;
  buf->f_bavail = 0;
  buf->f_files = 2;
  buf->f_ffree = 0;
  buf->f_favail = 0;
  buf->f_fsid = 0;
  buf->f_flag= 0;
  buf->f_namemax = 16;
  return 0;
}


int close(int fd)
noexcept(noexcept(close(fd)))
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

int access(const char *path, int mode)
noexcept(noexcept(access(path, mode)))
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
  extern "C" ret func ptlist noexcept(noexcept(func plist))     \
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
  extern "C" ret func ptlist noexcept(noexcept(func plist))     \
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
  extern "C" ret func ptlist noexcept(noexcept(func plist))     \
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
  extern "C" ret func ptlist noexcept(noexcept(func plist))     \
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
  extern "C" ret func ptlist noexcept(noexcept(func plist))     \
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
L4B_REDIRECT_4(int, fchmodat, int, const char *, mode_t, int)
L4B_REDIRECT_4(int, utimensat, int, const char *,
                               const struct timespec *, int)


// ------------------------------------------------------

#include <l4/util/util.h>
#include <l4/cxx/lock_guard.h>
#include <l4/cxx/minmax>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <poll.h>

/**
 * Condition variable for signaling file I/O operation/condition readiness.
 */
static pthread_mutex_t __fnotify_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t __fnotify_cv = PTHREAD_COND_INITIALIZER;

/**
 * System V user space signaling.
 *
 * This is a System V user space signaling facility that is deprecated and
 * unused, but still a de facto standard (thus it is safe to be declared in
 * signal.h).
 *
 * We piggy-back on the SIGURG signal (which is defined to be ignored by
 * default) to signal I/O operation/condition readiness.
 *
 * \param sig  User space signal to send.
 *
 * \return 0 on success, non-zero on failure.
 */
int gsignal(int sig) __THROW
{
  if (sig == SIGURG)
    {
      // Signal I/O operation/condition readiness.
      auto guard = L4::Lock_guard(__fnotify_mtx);
      assert(guard.status() == 0);

      pthread_cond_broadcast(&__fnotify_cv);
      return 0;
    }

  return -1;
}

/**
 * Check file descriptor I/O operation/condition readiness.
 *
 * Check whether the file associated with a file descriptor is ready for the
 * given I/O operation/condition readiness (if the file descriptor is a member
 * of the file descriptor input set).
 *
 * \param[in]     rt     Type of the I/O operation/condition to be ready
 *                       (#Read, #Write, #Exception).
 * \param[in]     fd     File descriptor to check.
 * \param[in]     file   Corresponding file object to check.
 * \param[in]     fds    File descriptor input set. Only if #fd is part of the
 *                       input set it is checked. If the argument is NULL, then
 *                       the input set is considered empty.
 * \param[out]    ofds   File descriptor output set. If #fd is ready, it is
 *                       marked the output set.
 * \param[in,out] ready  Total number of file descriptors that are ready.
 *                       Incremented if #fd is ready.
 *
 * \retval true   File descriptor checked or not a member of the input set.
 * \retval false  File descriptor is part of the input set, but it is invalid.
 */
static bool __internal_pselect_check(File::Ready_type rt,
                                     cxx::Ref_ptr<L4Re::Vfs::File> &file,
                                     int fd, fd_set *fds, fd_set &ofds,
                                     unsigned &ready)
{
  // Empty input set or non-member file descriptor.
  if (!fds || !FD_ISSET(fd, fds))
    return true;

  // Invalid file descriptor.
  if (!file)
    return false;

  if (file->check_ready(rt))
    {
      FD_SET(fd, &ofds);
      ++ready;
    }

  return true;
}

/**
 * Normalize struct timespec value.
 *
 * Make sure the nanosecond member of struct timespec is within the interval
 * [0, 999'999'999].
 *
 * \param[in,out] ts  Time specification to normalize.
 */
static void __internal_normalize_timespec(struct timespec &ts)
{
  // Convert overflow nanoseconds to seconds.
  if (ts.tv_nsec >= 1000000000L || ts.tv_nsec <= -1000000000L)
    {
      intmax_t delta = ts.tv_nsec / 1000000000L;
      ts.tv_sec += delta;
      ts.tv_nsec -= delta * 1000000000L;
    }

  // Make nanoseconds non-negative.
  if (ts.tv_nsec < 0)
    {
      ts.tv_sec -= 1;
      ts.tv_nsec += 1000000000L;
    }
}

/**
 * Convert relative timeout to absolute time.
 *
 * \param[in]  timeout  Relative timeout.
 * \param[out] ts       Absolute time.
 */
static void __internal_pselect_timespec(const struct timespec &timeout,
                                        struct timespec &ts)
{
  // We assume that clock_gettime(CLOCK_REALTIME, ...) never fails in L4Re.
  clock_gettime(CLOCK_REALTIME, &ts);

  // Convert relative timeout into absolute time.
  ts.tv_sec += timeout.tv_sec;
  ts.tv_nsec += timeout.tv_nsec;

  __internal_normalize_timespec(ts);
}

/**
 * Convert relative timeout to absolute time.
 *
 * \param[in]  timeout  Relative timeout (in ms).
 * \param[out] ts       Absolute time.
 */
static void __internal_poll_timespec(int timeout, struct timespec &ts)
{
  // We assume that clock_gettime(CLOCK_REALTIME, ...) never fails in L4Re.
  clock_gettime(CLOCK_REALTIME, &ts);

  // Convert relative timeout into absolute time.
  ts.tv_sec += timeout / 1000;
  ts.tv_nsec += (timeout % 1000) * 1000000;

  __internal_normalize_timespec(ts);
}

/**
 * Standard-compliant select() implementation.
 *
 * To comply with standards, only the first #FD_SETSIZE file descriptors are
 * checked.
 *
 * \param[in]     nfds       Upper bound of the file descriptors to check.
 * \param[in,out] readfds    Input set of file descriptors to check for read
 *                           readiness and output set of file descriptors ready
 *                           for read. Ignored if NULL.
 * \param[in,out] writefds   Input set of file descriptors to check for write
 *                           readiness and output set of file descriptors ready
 *                           for write. Ignored if NULL.
 * \param[in,out] exceptfds  Input set of file descriptors to check for
 *                           exception condition and output set of file
 *                           descriptors with an exception condition. Ignored
 *                           if NULL.
 * \param[in]     timeout    Waiting timeout. Wait without a timeout if NULL.
 *
 * \return Total number of ready file descriptors in all output sets if
 *         non-negative. -1 in case of an error (which is indicated in #errno).
 */
int select(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds,
           fd_set *__restrict exceptfds, struct timeval *__restrict timeout)
{
  if (timeout)
    {
      struct timespec ts;

      ts.tv_sec = timeout->tv_sec;
      ts.tv_nsec = timeout->tv_usec * 1000;

      return pselect(nfds, readfds, writefds, exceptfds, &ts, nullptr);
    }

  return pselect(nfds, readfds, writefds, exceptfds, nullptr, nullptr);
}

/**
 * Standard-compliant pselect() implementation.
 *
 * To comply with standards, only the first #FD_SETSIZE file descriptors are
 * checked.
 *
 * \note The atomic signal masking feature is not implemented yet (pending
 *       signal support).
 *
 * \param[in]     nfds       Upper bound of the file descriptors to check.
 * \param[in,out] readfds    Input set of file descriptors to check for read
 *                           readiness and output set of file descriptors ready
 *                           for read. Ignored if NULL.
 * \param[in,out] writefds   Input set of file descriptors to check for write
 *                           readiness and output set of file descriptors ready
 *                           for write. Ignored if NULL.
 * \param[in,out] exceptfds  Input set of file descriptors to check for
 *                           exception condition and output set of file
 *                           descriptors with an exception condition. Ignored
 *                           if NULL.
 * \param[in]     timeout    Waiting timeout. Wait without a timeout if NULL.
 * \param[in]     sigmask    Signals that are masked during the waiting.
 *                           Ignored if NULL. Currently not used.
 *
 * \return Total number of ready file descriptors in all output sets if
 *         non-negative. -1 in case of an error (which is indicated in #errno).
 */
int pselect(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds,
            fd_set *__restrict exceptfds,
            const struct timespec *__restrict timeout,
            [[maybe_unused]] const sigset_t *__restrict sigmask)
{
  if (nfds < 0)
    {
      errno = EINVAL;
      return -1;
    }

  bool poll = false;
  bool wtimeout = false;

  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;

  if (timeout)
    {
      // Zero timeout indicates a polling request.
      if (timeout->tv_sec == 0 && timeout->tv_nsec == 0)
        poll = true;
      else
        {
          __internal_pselect_timespec(*timeout, ts);
          wtimeout = true;
        }
    }

  fd_set oreadfds;
  fd_set owritefds;
  fd_set oexceptfds;
  unsigned ready;

  auto guard = L4::Lock_guard(__fnotify_mtx);
  assert(guard.status() == 0);

  while (true)
    {
      FD_ZERO(&oreadfds);
      FD_ZERO(&owritefds);
      FD_ZERO(&oexceptfds);
      ready = 0;

      for (int fd = 0; fd < cxx::min(nfds, FD_SETSIZE); ++fd)
        {
          auto file = vfs_ops->get_file(fd);

          if (!__internal_pselect_check(File::Ready_type::Read, file, fd, readfds,
                                        oreadfds, ready))
            {
              errno = EBADFD;
              return -1;
            }

          if (!__internal_pselect_check(File::Ready_type::Write, file, fd,
                                        writefds, owritefds, ready))
            {
              errno = EBADFD;
              return -1;
            }

          if (!__internal_pselect_check(File::Ready_type::Exception, file, fd,
                                        exceptfds, oexceptfds, ready))
            {
              errno = EBADFD;
              return -1;
            }
        }

      // Exit if at least one file descriptor is ready or polling is requested.
      if (ready > 0 || poll)
        break;

      // Wait for the I/O operation/condition readiness signal.
      if (wtimeout)
        {
          // Exit in case of a timeout or failure.
          if (pthread_cond_timedwait(&__fnotify_cv, &__fnotify_mtx, &ts) != 0)
            break;
        }
      else
        {
          // According to the specification, this should never fail.
          pthread_cond_wait(&__fnotify_cv, &__fnotify_mtx);
        }
    }

  if (readfds)
    *readfds = oreadfds;

  if (writefds)
    *writefds = owritefds;

  if (exceptfds)
    *exceptfds = oexceptfds;

  return ready;
}

/**
 * Check array of file descriptors I/O operation/condition readiness.
 *
 * Iteratively check whether the files associated with file descriptors in an
 * array are ready for the given I/O operation/condition readiness.
 *
 * \param[in,out] fds       Array of file descriptors.
 * \param[in]     nfds      Number of array members.
 * \param[in]     poll      Only run one iteration if true.
 * \param[in]     wtimeout  Wait with a timeout if true.
 * \param[in]     ts        Absolute time of timeout.
 *
 * \return Number of file descriptors that are ready.
 */
static unsigned __internal_ppoll(struct pollfd *fds, nfds_t nfds, bool poll,
                                 bool wtimeout, const struct timespec &ts)
{
  unsigned ready;

  auto guard = L4::Lock_guard(__fnotify_mtx);
  assert(guard.status() == 0);

  while (true)
    {
      ready = 0;

      for (nfds_t i = 0; i < nfds; ++i)
        {
          struct pollfd &cur = fds[i];
          cur.revents = 0;

          // Ignore negative file descriptors.
          if (cur.fd < 0)
            continue;

          auto file = vfs_ops->get_file(cur.fd);
          if (!file)
            {
              // Invalid file descriptor.
              cur.revents |= POLLNVAL;
              ++ready;
              continue;
            }

          if ((cur.events & POLLIN) != 0
               && file->check_ready(File::Ready_type::Read))
            cur.revents |= POLLIN;

          if ((cur.events & POLLOUT) != 0
               && file->check_ready(File::Ready_type::Write))
            cur.revents |= POLLOUT;

          if (file->check_ready(File::Ready_type::Exception))
            cur.revents |= POLLERR;

          if (cur.revents != 0)
            ++ready;
        }

      // Exit if at least one file descriptor is ready or polling is requested.
      if (ready > 0 || poll)
        break;

      // Wait for the I/O operation/condition readiness signal.
      if (wtimeout)
        {
          // Exit in case of a timeout or failure.
          if (pthread_cond_timedwait(&__fnotify_cv, &__fnotify_mtx, &ts) != 0)
            break;
        }
      else
        {
          // According to the specification, this should never fail.
          pthread_cond_wait(&__fnotify_cv, &__fnotify_mtx);
        }
    }

  return ready;
}

/**
 * Standard-compliant poll() implementation.
 *
 * \param[in,out] fds      Array of file descriptors.
 * \param[in]     nfds     Number of array members.
 * \param[in]     timeout  Waiting timeout. Wait without a timeout if negative.
 *                         Return immediately after the first check if zero.
 *
 * \return Number of file descriptors that are ready.
 */
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  bool poll = false;
  bool wtimeout = false;

  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;

  if (timeout == 0)
    poll = true;
  else if (timeout > 0)
    {
      __internal_poll_timespec(timeout, ts);
      wtimeout = true;
    }

  return __internal_ppoll(fds, nfds, poll, wtimeout, ts);
}

/**
 * Standard-compliant ppoll() implementation.
 *
 * \note The atomic signal masking feature is not implemented yet (pending
 *       signal support).
 *
 * \param[in,out] fds      Array of file descriptors.
 * \param[in]     nfds     Number of array members.
 * \param[in]     timeout  Waiting timeout. Wait without a timeout if NULL.
 * \param[in]     sigmask  Signals that are masked during the waiting. Ignored
 *                         if NULL. Currently not used.
 *
 * \return Number of file descriptors that are ready.
 */
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
          [[maybe_unused]] const sigset_t *sigmask)
{
  bool poll = false;
  bool wtimeout = false;

  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;

  if (timeout)
    {
      // Zero timeout indicates a polling request.
      if (timeout->tv_sec == 0 && timeout->tv_nsec == 0)
        poll = true;
      else
        {
          __internal_pselect_timespec(*timeout, ts);
          wtimeout = true;
        }
    }

  return __internal_ppoll(fds, nfds, poll, wtimeout, ts);
}

#undef L4B_REDIRECT

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  ret func ptlist noexcept(noexcept(func plist)) \
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
ssize_t preadv(int, const struct iovec *, int, __off64_t);
ssize_t pwritev(int, const struct iovec *, int, __off64_t);
__END_DECLS

L4B_REDIRECT_2(int,       fstat64,     int, struct stat64 *)
L4B_REDIRECT_3(ssize_t,   readv,       int, const struct iovec *, int)
L4B_REDIRECT_3(ssize_t,   writev,      int, const struct iovec *, int)
L4B_REDIRECT_4(ssize_t,   preadv,      int, const struct iovec *, int, __off64_t)
L4B_REDIRECT_4(ssize_t,   pwritev,     int, const struct iovec *, int, __off64_t)
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

extern "C" int chdir(const char *path) noexcept(noexcept(chdir(path)))
{
  Ref_ptr<File> f;
  int res = __internal_resolve(AT_FDCWD, path, 0, 0, &f);
  ERRNO_RET(res);

  if (*path == '/')
    {
      char *new_cwd = strdup(path);
      if (!new_cwd)
        {
          errno = ENOMEM;
          return -1;
        }
      free_cwd();
      _current_working_dir = new_cwd;
    }
  else
    {
      unsigned len_cwd = strlen(_current_working_dir);
      unsigned len_path = strlen(path);
      char *new_cwd = static_cast<char *>(malloc(len_cwd + len_path + 2));
      if (!new_cwd)
        {
          errno = ENOMEM;
          return -1;
        }
      memcpy(new_cwd, _current_working_dir, len_cwd);
      if (new_cwd[len_cwd - 1] != '/')
        new_cwd[len_cwd++] = '/';
      memcpy(new_cwd + len_cwd, path, len_path + 1);

      free_cwd();
      _current_working_dir = new_cwd;
    }

  // would need to check whether 'f' is a directory
  L4Re::Vfs::vfs_ops->set_cwd(f);

  return 0;
}

extern "C" int fchdir(int fd) noexcept(noexcept(fchdir(fd)))
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
  return 0;
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

extern "C" char *getcwd(char *buf, size_t size)
noexcept(noexcept(getcwd(buf, size)))
{
  unsigned len_cwd = strlen(_current_working_dir) + 1;

  /* Posix mandates returning EINVAL if a buffer is supplied without a size */
  if (buf != 0 && size == 0)
    {
      errno = EINVAL;
      return 0;
    }

  if (buf == 0 && size == 0)
    size = len_cwd;

  if (len_cwd > size)
    {
      errno = ERANGE;
      return 0;
    }

  if (buf == 0)
    buf = static_cast<char *>(malloc(size));

  if (buf == 0)
    {
      errno = ENOMEM;
      return 0;
    }

  memcpy(buf, _current_working_dir, len_cwd);
  return buf;
}

extern "C" int chroot(const char *p)
noexcept(noexcept(chroot(p)))
{
  errno = EIO;
  return -1;
}

extern "C" int mkfifo(const char *p, mode_t m)
noexcept(noexcept(mkfifo(p, m)))
{
  /* mkfifo on Linux returns EPERM on unsupported file systems */
  errno = EPERM;
  return -1;
}

extern "C" int mknod(const char *p, mode_t m, dev_t d)
noexcept(noexcept(mknod(p, m, d)))
{
  errno = EINVAL;
  return -1;
}

int chown(const char *, uid_t, gid_t) __THROW
{
  errno = EINVAL;
  return -1;
}

int fchown(int, uid_t, gid_t) __THROW
{
  errno = EINVAL;
  return -1;
}


extern "C" int lchown(const char *p, uid_t u, gid_t g)
noexcept(noexcept(lchown(p, u, g)))
{
  errno = EINVAL;
  return -1;
}
