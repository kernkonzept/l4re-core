#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

static inline void
_dl_exit(int status) { exit(status); }

static inline int
_dl_mprotect(void *addr, size_t len, int prot)
{ (void)addr; (void)len; (void)prot; return -1; } //mprotect(addr, len, prot); }

static inline int
_dl_stat(const char *path, struct stat * buf)
{ return stat(path, buf); }

static inline int
_dl_fstat(int fd, struct stat *buf)
{ return fstat(fd, buf); }

static inline int
_dl_getuid(void)
{ return 0; }

 static inline int
_dl_geteuid(void)
{ return 0; }

static inline int
_dl_getgid(void)
{ return 0; }

static inline int
_dl_getegid(void)
{ return 0; }



static inline int
_dl_munmap(void *start, size_t len)
{ return munmap(start, len); }

static inline int
_dl_mmap_check_error(void *x)
{ return x == MAP_FAILED; }

extern int open (const char *__file, int __oflag, ...) __nonnull ((1));

static inline int
_dl_open(char const *path, int flags, int mode)
{ return open(path, flags, mode); }

static inline int
_dl_close(int fd)
{ return close(fd); }

static inline int
_dl_read(int fd, void *buf, size_t len)
{ return read(fd, buf, len); }

static inline int
_dl_write(int fd, char const *str, size_t len)
{ (void)fd; (void)str; (void)len; return -1; }

static inline int
_dl_getpid(void)
{ return 10; }

static inline int
_dl_cap_equal(unsigned long a, unsigned long b)
{
  (void)a;
  (void)b;
  return 0;
}


static inline void *
_dl_mmap(void *start, size_t len, int prot, int flags, int fd, off_t offset)
{ return mmap(start, len, prot, flags, fd, offset); }
#ifdef SHARED
#define _dl_errno errno
#else
extern int _dl_errno;
#endif
