
#include <l4/l4re_vfs/vfs.h>
#include <l4/sys/kdebug.h>
#include <l4/re/env>

using namespace L4Re::Vfs;
using cxx::Ref_ptr;

#define L4RE_CALL(call...) \
extern "C" attribute_hidden call; \
extern "C" attribute_hidden call

extern "C" attribute_hidden int _dl_open(char const *path, int flags, int mode);
extern "C" attribute_hidden int _dl_close(int fd);
extern "C" attribute_hidden L4Re::Vfs::Ops *__rtld_l4re_env_posix_vfs_ops;

#define L4RE_VFS __rtld_l4re_env_posix_vfs_ops

static inline
void
_dl_convert_stat(struct stat *buf, struct stat64 const &s)
{
  buf->st_dev = s.st_dev;
  buf->st_ino = s.st_ino;
  buf->st_mode = s.st_mode;
  buf->st_nlink = s.st_nlink;
  buf->st_uid = s.st_uid;
  buf->st_gid = s.st_gid;
  buf->st_size = s.st_size;
}

L4RE_CALL(int _dl_stat(const char *p, struct stat *buf))
{
  Ref_ptr<File> f;
  int res = L4RE_VFS->get_root()->openat(p, 0, 0, &f);

  if (res < 0)
    return res;

  struct stat64 s;
  res = f->fstat64(&s);

  if (res < 0)
    return res;

  _dl_convert_stat(buf, s);
  return res;
}


L4RE_CALL(int _dl_fstat(int fd, struct stat *buf))
{
  Ref_ptr<File> f = L4RE_VFS->get_file(fd);

  if (!f)
    return -EBADF;

  struct stat64 s;
  int res = f->fstat64(&s);

  if (res < 0)
    return res;

  _dl_convert_stat(buf, s);
  return res;
}


L4RE_CALL(int _dl_read(int fd, void *buf, size_t len))
{
  Ref_ptr<File> f = L4RE_VFS->get_file(fd);

  if (!f)
    return -EBADF;

  struct iovec v;
  v.iov_len = len;
  v.iov_base = buf;
#if 0
  outstring("### DL: read: "); outhex32(fd); outstring("\n");
#endif
  int res = f->readv(&v, 1);
  return res;
};


L4RE_CALL(int _dl_close(int fd))
{
  Ref_ptr<File> f = L4RE_VFS->free_fd(fd);
  if (!f)
    return -EBADF;

  f->unlock_all_locks();
  return 0;
}


L4RE_CALL(int _dl_open(char const *path, int flags, int mode))
{ //outstring("### DL: open: '"); outstring(path); outstring ("'\n");
#if 0
  if (!_dl_init_auxvp())
    return -L4_ENOSYS;
  return __rtld_l4re_env_posix_vfs_ops->open(path, flags, mode);
#endif

  Ref_ptr<File> f;
  int res = L4RE_VFS->get_root()->openat(path, flags, mode, &f);

  if (res < 0)
    return res;

  int fd = L4RE_VFS->alloc_fd(f);

  return fd;
}

L4RE_CALL(int _dl_write(int fd, char const *str, size_t len))
{
  Ref_ptr<File> f = L4RE_VFS->get_file(fd);

  if (!f)
    return -EBADF;

  struct iovec v;
  v.iov_len = len;
  v.iov_base = (char*)str;

  if (len <= 0)
    return 0;

  int res = f->writev(&v, 1);
  return res;
}



L4RE_CALL(int _dl_mprotect(void const *addr, size_t len, int prot))
{
  return L4RE_VFS->mprotect((void *)addr, len, prot);
}




L4RE_CALL(int _dl_munmap(void *start, size_t len))
{
  return L4RE_VFS->munmap(start, len);
}

L4RE_CALL(void *_dl_mmap(void *start, size_t len, int prot, int flags, int fd, off_t offset))
{
#if 0
  outstring("### DL: _dl_mmap(");
    outhex32((int)start); outstring(", ");
    outhex32((int)len); outstring(", ");
    outhex32((int)prot); outstring(", ");
    outhex32((int)flags); outstring(", ");
    outhex32((int)fd); outstring(", ");
    outhex32((int)offset); outstring("); from=");
  outhex32((int)__builtin_return_address(0)); outstring("\n");
#endif
  void *resptr;
  int res = L4RE_VFS->mmap2(start, len, prot, flags, fd, offset >> 12, &resptr);
  if (res >= 0)
    return resptr;
  else
    return MAP_FAILED;
}
