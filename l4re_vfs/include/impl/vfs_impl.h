/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include "fd_store.h"
#include "vcon_stream.h"
#include "ns_fs.h"

#include <l4/bid_config.h>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/sys/assert.h>
#include <l4/cxx/hlist>
#include <l4/cxx/pair>
#include <l4/cxx/std_alloc>

#include <l4/l4re_vfs/backend>
#include <l4/re/shared_cap>

#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/mman.h>

#if 0
#include <l4/sys/kdebug.h>
static int debug_mmap = 1;
#define DEBUG_LOG(level, dbg...) do { if (level) dbg } while (0)
#else
#define DEBUG_LOG(level, dbg...) do { } while (0)
#endif

/**
 * If USE_BIG_ANON_DS is defined the implementation will use a really big
 * data space for backing anonymous memory. Otherwise each mmap call
 * with anonymous memory will allocate a separate data space.
 */
#define USE_BIG_ANON_DS

using L4Re::Rm;

namespace {

using cxx::Ref_ptr;

class Fd_store : public L4Re::Core::Fd_store
{
public:
  Fd_store() noexcept;
};

// for internal Vcon_streams we want to have a placement new operator, so
// inherit and add one
class Std_stream : public L4Re::Core::Vcon_stream
{
public:
  Std_stream(L4::Cap<L4::Vcon> c) : L4Re::Core::Vcon_stream(c) {}
};

Fd_store::Fd_store() noexcept
{
  // use this strange way to prevent deletion of the stdio object
  // this depends on Fd_store to being a singleton !!!
  static char m[sizeof(Std_stream)] __attribute__((aligned(sizeof(long))));
  if (auto log = L4Re::Env::env()->log())
    {
      Std_stream *s = new (m) Std_stream(log);
      set(0, cxx::ref_ptr(s)); // stdin
      set(1, cxx::ref_ptr(s)); // stdout
      set(2, cxx::ref_ptr(s)); // stderr

      // make sure that we never delete the static io stream thing
      s->add_ref();
    }
}

class Root_mount_tree : public L4Re::Vfs::Mount_tree
{
public:
  Root_mount_tree() : L4Re::Vfs::Mount_tree(0) {}
  void operator delete (void *) {}
};

class Vfs : public L4Re::Vfs::Ops
{
private:
  bool _early_oom;

public:
  Vfs()
  : _early_oom(true), _root_mount(), _root(L4Re::Env::env())
  {
    _root_mount.add_ref();
    _root.add_ref();
    _root_mount.mount(cxx::ref_ptr(&_root));
    _cwd = cxx::ref_ptr(&_root);

#if 0
    Ref_ptr<L4Re::Vfs::File> rom;
    _root.openat("rom", 0, 0, &rom);

    _root_mount.create_tree("lib/foo", rom);

    _root.openat("lib", 0, 0, &_cwd);

#endif
  }

  int alloc_fd(Ref_ptr<L4Re::Vfs::File> const &f) noexcept override;
  Ref_ptr<L4Re::Vfs::File> free_fd(int fd) noexcept override;
  Ref_ptr<L4Re::Vfs::File> get_root() noexcept override;
  Ref_ptr<L4Re::Vfs::File> get_cwd() noexcept override;
  void set_cwd(Ref_ptr<L4Re::Vfs::File> const &dir) noexcept override;
  Ref_ptr<L4Re::Vfs::File> get_file(int fd) noexcept override;
  cxx::Pair<Ref_ptr<L4Re::Vfs::File>, int>
    set_fd(int fd, Ref_ptr<L4Re::Vfs::File> const &f = Ref_ptr<>::Nil) noexcept
      override;

  int mmap2(void *start, size_t len, int prot, int flags, int fd,
            off_t offset, void **ptr) noexcept override;

  int munmap(void *start, size_t len) noexcept override;
  int mprotect(const void *a, size_t sz, int prot) noexcept override;
  int msync(void *addr, size_t len, int flags) noexcept override;
  int madvise(void *addr, size_t len, int advice) noexcept override;

  int register_file_system(L4Re::Vfs::File_system *f) noexcept override;
  int unregister_file_system(L4Re::Vfs::File_system *f) noexcept override;
  L4Re::Vfs::File_system *get_file_system(char const *fstype) noexcept override;
  L4Re::Vfs::File_system_list file_system_list() noexcept override;

  int register_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) noexcept override;
  int unregister_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) noexcept override;
  Ref_ptr<L4Re::Vfs::File_factory> get_file_factory(int proto) noexcept override;
  Ref_ptr<L4Re::Vfs::File_factory> get_file_factory(char const *proto_name) noexcept override;
  int mount(char const *path, cxx::Ref_ptr<L4Re::Vfs::File> const &dir) noexcept override;

  void operator delete (void *) {}

  void *malloc(size_t size) noexcept override { return Vfs_config::malloc(size); }
  void free(void *m) noexcept override { Vfs_config::free(m); }

private:
  Root_mount_tree _root_mount;
  L4Re::Core::Env_dir _root;
  Ref_ptr<L4Re::Vfs::File> _cwd;
  Fd_store fds;

  L4Re::Vfs::File_system *_fs_registry;

  struct File_factory_item : cxx::H_list_item_t<File_factory_item>
  {
    cxx::Ref_ptr<L4Re::Vfs::File_factory> f;
    explicit File_factory_item(cxx::Ref_ptr<L4Re::Vfs::File_factory> const &f)
    : f(f) {};

    File_factory_item() = default;
    File_factory_item(File_factory_item const &) = delete;
    File_factory_item &operator = (File_factory_item const &) = delete;
  };

  cxx::H_list_t<File_factory_item> _file_factories;

  void align_mmap_start_and_length(void **start, size_t *length);
  int munmap_regions(void *start, size_t len);

  L4Re::Vfs::File_system *find_fs_from_type(char const *fstype) noexcept;
};

static inline bool strequal(char const *a, char const *b)
{
  for (;*a && *a == *b; ++a, ++b)
    ;
  return *a == *b;
}

int
Vfs::register_file_system(L4Re::Vfs::File_system *f) noexcept
{
  using L4Re::Vfs::File_system;

  if (!f)
    return -EINVAL;

  for (File_system *c = _fs_registry; c; c = c->next())
    if (strequal(c->type(), f->type()))
      return -EEXIST;

  f->next(_fs_registry);
  _fs_registry = f;

  return 0;
}

int
Vfs::unregister_file_system(L4Re::Vfs::File_system *f) noexcept
{
  using L4Re::Vfs::File_system;

  if (!f)
    return -EINVAL;

  File_system **p = &_fs_registry;

  for (; *p; p = &(*p)->next())
    if (*p == f)
      {
        *p = f->next();
        f->next() = 0;
        return 0;
      }

  return -ENOENT;
}

L4Re::Vfs::File_system *
Vfs::find_fs_from_type(char const *fstype) noexcept
{
  L4Re::Vfs::File_system_list fsl(_fs_registry);
  for (L4Re::Vfs::File_system_list::Iterator c = fsl.begin();
       c != fsl.end(); ++c)
    if (strequal(c->type(), fstype))
      return *c;
  return 0;
}

L4Re::Vfs::File_system_list
Vfs::file_system_list() noexcept
{
  return L4Re::Vfs::File_system_list(_fs_registry);
}

L4Re::Vfs::File_system *
Vfs::get_file_system(char const *fstype) noexcept
{
  L4Re::Vfs::File_system *fs;
  if ((fs = find_fs_from_type(fstype)))
    return fs;

  // Try to load a file system module dynamically
  int res = Vfs_config::load_module(fstype);
  if (res < 0)
    return 0;

  // Try again
  return find_fs_from_type(fstype);
}

int
Vfs::register_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) noexcept
{
  if (!f)
    return -EINVAL;

  void *x = this->malloc(sizeof(File_factory_item));
  if (!x)
    return -ENOMEM;

  auto ff = new (x, cxx::Nothrow()) File_factory_item(f);
  _file_factories.push_front(ff);
  return 0;
}

int
Vfs::unregister_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) noexcept
{
  for (auto p: _file_factories)
    {
      if (p->f == f)
        {
          _file_factories.remove(p);
          p->~File_factory_item();
          this->free(p);
          return 0;
        }
    }
  return -ENOENT;
}

Ref_ptr<L4Re::Vfs::File_factory>
Vfs::get_file_factory(int proto) noexcept
{
  for (auto p: _file_factories)
    if (p->f->proto() == proto)
      return p->f;

  return Ref_ptr<L4Re::Vfs::File_factory>();
}

Ref_ptr<L4Re::Vfs::File_factory>
Vfs::get_file_factory(char const *proto_name) noexcept
{
  for (auto p: _file_factories)
    {
      auto n = p->f->proto_name();
      if (n)
        {
          char const *a = n;
          char const *b = proto_name;
          for (; *a && *b && *a == *b; ++a, ++b)
            ;

          if ((*a == 0) && (*b == 0))
            return p->f;
        }
    }

  return Ref_ptr<L4Re::Vfs::File_factory>();
}

int
Vfs::alloc_fd(Ref_ptr<L4Re::Vfs::File> const &f) noexcept
{
  int fd = fds.alloc();
  if (fd < 0)
    return -EMFILE;

  if (f)
    fds.set(fd, f);

  return fd;
}

Ref_ptr<L4Re::Vfs::File>
Vfs::free_fd(int fd) noexcept
{
  Ref_ptr<L4Re::Vfs::File> f = fds.get(fd);

  if (!f)
    return Ref_ptr<>::Nil;

  fds.free(fd);
  return f;
}


Ref_ptr<L4Re::Vfs::File>
Vfs::get_root() noexcept
{
  return cxx::ref_ptr(&_root);
}

Ref_ptr<L4Re::Vfs::File>
Vfs::get_cwd() noexcept
{
  return _cwd;
}

void
Vfs::set_cwd(Ref_ptr<L4Re::Vfs::File> const &dir) noexcept
{
  // FIXME: check for is dir
  if (dir)
    _cwd = dir;
}

Ref_ptr<L4Re::Vfs::File>
Vfs::get_file(int fd) noexcept
{
  return fds.get(fd);
}

cxx::Pair<Ref_ptr<L4Re::Vfs::File>, int>
Vfs::set_fd(int fd, Ref_ptr<L4Re::Vfs::File> const &f) noexcept
{
  if (!fds.check_fd(fd))
    return cxx::pair(Ref_ptr<L4Re::Vfs::File>(Ref_ptr<>::Nil), EBADF);

  Ref_ptr<L4Re::Vfs::File> old = fds.get(fd);
  fds.set(fd, f);
  return cxx::pair(old, 0);
}


#define GET_FILE_DBG(fd, err) \
  Ref_ptr<L4Re::Vfs::File> fi = fds.get(fd); \
  if (!fi)                           \
    {                                \
      return -err;                   \
    }

#define GET_FILE(fd, err) \
  Ref_ptr<L4Re::Vfs::File> fi = fds.get(fd); \
  if (!fi)                           \
    return -err;

void
Vfs::align_mmap_start_and_length(void **start, size_t *length)
{
  l4_addr_t const s = reinterpret_cast<l4_addr_t>(*start);
  size_t const o = s & (L4_PAGESIZE - 1);

  *start   = reinterpret_cast<void *>(l4_trunc_page(s));
  *length  = l4_round_page(*length + o);
}

int
Vfs::munmap_regions(void *start, size_t len)
{
  using namespace L4;
  using namespace L4Re;

  int err;
  Cap<Rm> r = Env::env()->rm();

  if (l4_addr_t(start) & (L4_PAGESIZE - 1))
    return -EINVAL;

  align_mmap_start_and_length(&start, &len);

  while (1)
    {
      DEBUG_LOG(debug_mmap, {
                l4_kd_outstring("DETACH: start = 0x");
                l4_kd_outhex32(l4_addr_t(start));
                l4_kd_outstring(" len = 0x");
                l4_kd_outhex32(len);
                l4_kd_outstring("\n");
      });
      err = r->detach(l4_addr_t(start), len, nullptr, This_task);
      if (err < 0)
        return err;

      if (!(err & Rm::Detach_again))
        return 0;
    }
}

int
Vfs::munmap(void *start, size_t len) L4_NOTHROW
{
  using namespace L4;
  using namespace L4Re;

  int err = 0;
  Cap<Rm> r = Env::env()->rm();

  // Fields for obtaining a list of areas for the calling process
  long area_cnt = -1;           // No. of areas in this process
  Rm::Area const *area_array;
  bool matches_area = false;        // true if unmap parameters match an area

  // First check if there are any areas matching the munmap request. Those
  // might have been created by an mmap call using PROT_NONE as protection
  // modifier.

  area_cnt = r->get_areas((l4_addr_t) start, &area_array);

  // It is enough to check for the very first entry, since get_areas will
  // only return areas with a starting address equal or greater to <start>.
  // However, we intend to unmap at most the area starting exactly at
  // <start>.
  if (area_cnt > 0)
    {
      size_t area_size = area_array[0].end - area_array[0].start + 1;

      // Only free the area if the munmap parameters describe it exactly.
      if (area_array[0].start == (l4_addr_t) start && area_size == len)
        {
          r->free_area((l4_addr_t) start);
          matches_area = true;
        }
    }

  // After clearing possible area reservations from PROT_NONE mappings, clear
  // any regions in the address range specified. Note that errors shall be
  // suppressed if an area was freed but no regions were found.
  err = munmap_regions(start, len);
  if (err == -ENOENT && matches_area)
    return 0;

  return err;
}

int
Vfs::mmap2(void *start, size_t len, int prot, int flags, int fd, off_t page4k_offset,
           void **resptr) L4_NOTHROW
{
  DEBUG_LOG(debug_mmap, {
            l4_kd_outstring("MMAP params: ");
            l4_kd_outstring("start = 0x");
            l4_kd_outhex32(l4_addr_t(start));
            l4_kd_outstring(", len = 0x");
            l4_kd_outhex32(len);
            l4_kd_outstring(", prot = 0x");
            l4_kd_outhex32(prot);
            l4_kd_outstring(", flags = 0x");
            l4_kd_outhex32(flags);
            l4_kd_outstring(", offset = 0x");
            l4_kd_outhex32(page4k_offset);
            l4_kd_outstring("\n");
  });

  using namespace L4Re;
  off64_t offset = l4_trunc_page(page4k_offset << 12);

  if (flags & MAP_FIXED)
    if (l4_addr_t(start) & (L4_PAGESIZE - 1))
      return -EINVAL;

  align_mmap_start_and_length(&start, &len);

  // special code to just reserve an area of the virtual address space
  // Same behavior should be exposed when mapping with PROT_NONE. Mind that
  // PROT_NONE can only be specified exclusively, since it is defined to 0x0.
  if ((flags & 0x1000000) || (prot == PROT_NONE))
    {
      int err;
      L4::Cap<Rm> r = Env::env()->rm();
      l4_addr_t area = reinterpret_cast<l4_addr_t>(start);
      err = r->reserve_area(&area, len, L4Re::Rm::F::Search_addr);
      if (err < 0)
        return err;

      *resptr = reinterpret_cast<void*>(area);

      DEBUG_LOG(debug_mmap, {
                l4_kd_outstring("  MMAP reserved area: 0x");
                l4_kd_outhex32(area);
                l4_kd_outstring("  length= 0x");
                l4_kd_outhex32(len);
                l4_kd_outstring("\n");
      });

      return 0;
    }

  L4::Cap<L4Re::Dataspace> ds;
  L4Re::Rm::Flags rm_flags(0);
  char const *region_name = "[unknown]";
  l4_addr_t file_offset = 0;

  if (flags & MAP_PRIVATE)
    rm_flags |= L4Re::Rm::F::Private;

  if (flags & MAP_ANONYMOUS)
    {
      rm_flags |= L4Re::Rm::F::Anonymous;
      offset = 0; // Or should we return EINVAL?
      region_name = "[anon]";
    }
  else
    {
      Ref_ptr<L4Re::Vfs::File> fi = fds.get(fd);
      if (!fi)
        return -EBADF;

      region_name = fi->path();

      ds = fi->data_space();
      if (!ds.is_valid())
        return -EINVAL;

      if (len + offset > l4_round_page(ds->size()))
        return -EINVAL;
    }


  if (!(flags & MAP_FIXED) && start == 0)
    start = reinterpret_cast<void*>(L4_PAGESIZE);

  char *data = static_cast<char *>(start);
  L4::Cap<Rm> r = Env::env()->rm();
  l4_addr_t overmap_area = L4_INVALID_ADDR;

  int err;
  if (flags & MAP_FIXED)
    {
      overmap_area = l4_addr_t(start);

      err = r->reserve_area(&overmap_area, len);
      if (err < 0)
        overmap_area = L4_INVALID_ADDR;

      rm_flags |= Rm::F::In_area;

      // Make sure to remove old mappings residing at the respective address 
      // range. If none exists, we are fine as well, allowing us to ignore
      // ENOENT here.
      err = munmap_regions(start, len);
      if (err && err != -ENOENT)
        return err;
    }

  if (!(flags & MAP_FIXED))
    rm_flags |= Rm::F::Search_addr;
  if (prot & PROT_READ)
    rm_flags |= Rm::F::R;
  if (prot & PROT_WRITE)
    rm_flags |= Rm::F::W;
  if (prot & PROT_EXEC)
    rm_flags |= Rm::F::X;

  // Region manager takes a reference to the ds cap...
  err = r->attach(&data, len, rm_flags,
                  L4::Ipc::make_cap(ds, (prot & PROT_WRITE)
                                        ? L4_CAP_FPAGE_RW
                                        : L4_CAP_FPAGE_RO),
                  offset, L4_PAGESHIFT, L4::Cap<L4::Task>::Invalid,
                  region_name, file_offset);

  DEBUG_LOG(debug_mmap, {
            l4_kd_outstring("  MAPPED: 0x");
            l4_kd_outhex32(ds.cap());
            l4_kd_outstring("  addr: 0x");
            l4_kd_outhex32(l4_addr_t(data));
            l4_kd_outstring("  bytes: 0x");
            l4_kd_outhex32(len);
            l4_kd_outstring("  offset: 0x");
            l4_kd_outhex32(offset);
            l4_kd_outstring("  err = ");
            l4_kd_outdec(err);
            l4_kd_outstring("\n");
  });


  if (overmap_area != L4_INVALID_ADDR)
    r->free_area(overmap_area);

  if (err < 0)
    return err;

  l4_assert (!(start && !data));

  *resptr = data;

  return 0;
}

int
Vfs::mprotect(const void * /* a */, size_t /* sz */, int prot) L4_NOTHROW
{
  return (prot & PROT_WRITE) ? -ENOSYS : 0;
}

int
Vfs::msync(void *, size_t, int) L4_NOTHROW
{ return 0; }

int
Vfs::madvise(void *, size_t, int) L4_NOTHROW
{ return 0; }

}

L4Re::Vfs::Ops *__rtld_l4re_env_posix_vfs_ops;
extern void *l4re_env_posix_vfs_ops __attribute__((alias("__rtld_l4re_env_posix_vfs_ops"), visibility("default")));

namespace {
  class Real_mount_tree : public L4Re::Vfs::Mount_tree
  {
  public:
    explicit Real_mount_tree(char *n) : Mount_tree(n) {}

    void *operator new (size_t size)
    { return __rtld_l4re_env_posix_vfs_ops->malloc(size); }

    void operator delete (void *mem)
    { __rtld_l4re_env_posix_vfs_ops->free(mem); }
  };
}

/// \relates L4Re::Vfs::Fs
int
Vfs::mount(char const *path, cxx::Ref_ptr<L4Re::Vfs::File> const &dir) noexcept
{
  using L4Re::Vfs::File;
  using L4Re::Vfs::Mount_tree;
  using L4Re::Vfs::Path;

  cxx::Ref_ptr<Mount_tree> root = get_root()->mount_tree();
  if (!root)
    return -EINVAL;

  cxx::Ref_ptr<Mount_tree> base;
  Path p = root->lookup(Path(path), &base);

  while (!p.empty())
    {
      Path f = p.strip_first();

      if (f.empty())
        return -EEXIST;

      char *name = __rtld_l4re_env_posix_vfs_ops->strndup(f.path(), f.length());
      if (!name)
        return -ENOMEM;

      auto nt = cxx::make_ref_obj<Real_mount_tree>(name);
      if (!nt)
        {
          __rtld_l4re_env_posix_vfs_ops->free(name);
          return -ENOMEM;
        }

      base->add_child_node(nt);
      base = nt;

      if (p.empty())
        {
          nt->mount(dir);
          return 0;
        }
    }

  return -EINVAL;
}

#undef DEBUG_LOG
#undef GET_FILE_DBG
#undef GET_FILE
