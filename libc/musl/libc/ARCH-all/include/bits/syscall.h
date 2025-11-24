#pragma once

#define __NEED_ssize_t
#define __NEED_size_t
#define __NEED_off_t
#define __NEED_uintptr_t
#include <bits/alltypes.h>
#include <errno.h>

struct iovec;

#ifdef __cplusplus
extern "C" {
#endif

// Musl checks for presence of certain syscalls with #ifdef.
#define SYS_mmap2
#define SYS_readlink
#define SYS_rename
#define SYS_rmdir
#define SYS_unlink

int __l4re_syscall_SYS_open(const char *path, int flags, ...);
int __l4re_syscall_SYS_close(int fd);
ssize_t __l4re_syscall_SYS_read(int fd, void *buf, size_t count);
ssize_t __l4re_syscall_SYS_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t __l4re_syscall_SYS_writev(int fd, const struct iovec *iov, int iovcnt);
int __l4re_syscall_SYS_fcntl(int fd, int op, ...);
int __l4re_syscall_SYS_ioctl(int fd, int op, ...);
int __l4re_syscall_SYS_rename(const char *oldpath, const char *newpath);
int __l4re_syscall_SYS_rmdir(const char *path);
ssize_t __l4re_syscall_SYS_readlink(const char *__restrict path, char *__restrict buf, size_t bufsiz);
int __l4re_syscall_SYS_unlink(const char *path);
int __l4re_syscall_SYS_pause(void);
void *__l4re_syscall_SYS_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int __l4re_syscall_SYS_munmap(void *addr, size_t len);
void *__l4re_syscall_SYS_mmap2(void *addr, size_t len, int prot, int flags, int fd, off_t offset) __attribute__((nothrow));
void *__l4re_syscall_SYS_mremap(void *old_addr, size_t old_size, size_t new_size, int flags, ...);
int __l4re_syscall_SYS_mprotect(void *addr, size_t len, int prot);

#ifdef __cplusplus
}
#endif

#define syscall(sc, ...) __libcall_ret((long)__l4re_syscall_ ## sc (__VA_ARGS__))

// Just aliasing the syscalls to our implementation would not be entirely
// correct, because the functions set `errno` while the syscall returns the
// actual error code. So we wrap the call and translate errno to return value.
static inline long __libcall_ret(long ret)
{
  return (ret == -1) ? -errno : ret;
}

#define __syscall syscall
#define syscall_cp syscall

#define __sys_open(...) __syscall(SYS_open, __VA_ARGS__)

// They can be called directly, since they are wrapped in syscall_ret in musl
#define sys_open(...) ((long)__l4re_syscall_SYS_open(__VA_ARGS__))
#define sys_pause() ((long)__l4re_syscall_SYS_pause())
