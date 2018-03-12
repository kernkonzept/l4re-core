#pragma once

#include <l4/sys/ipc.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/err.h>
#include <l4/sys/task.h>
#include <l4/re/consts.h>
#include <l4/re/elf_aux.h>

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

extern int _dl_errno;
extern attribute_hidden void *__rtld_l4re_env_posix_vfs_ops;
#define L4RE_VFS ((l4re_posix_vfs_ops_t *)__rtld_l4re_env_posix_vfs_ops)

#define L4RE_CALL(call...) \
attribute_hidden call

static inline void
_dl_exit(int status)
{ (void)status; while(1) ;/*l4_ipc_sleep(L4_IPC_NEVER);*/ }

static inline int
_dl_mmap_check_error(void *x)
{ return x == MAP_FAILED; }

L4RE_CALL(int _dl_mprotect(void const *addr, size_t len, int prot));
L4RE_CALL(int _dl_stat(const char *path, struct stat * buf));
L4RE_CALL(int _dl_fstat(int fd, struct stat *buf));
L4RE_CALL(int _dl_munmap(void *start, size_t len));
L4RE_CALL(int _dl_open(char const *path, int flags, int mode));
L4RE_CALL(int _dl_close(int fd));
L4RE_CALL(int _dl_read(int fd, void *buf, size_t len));
L4RE_CALL(int _dl_write(int fd, char const *str, size_t len));
L4RE_CALL(void *_dl_mmap(void *start, size_t len, int prot, int flags, int fd, off_t offset));


static inline int
_dl_cap_equal(unsigned long a, unsigned long b)
{
  l4_msgtag_t t = l4_task_cap_equal_u(L4RE_THIS_TASK_CAP, a, b, l4_utcb_direct());
  return l4_msgtag_label(t);
}

static inline int
_dl_getpid(void)
{ return 10; }

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



