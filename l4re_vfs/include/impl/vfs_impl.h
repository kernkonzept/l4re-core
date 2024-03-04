/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include "fd_store.h"
#include "vcon_stream.h"
#include "ns_fs.h"

#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/cxx/hlist>
#include <l4/cxx/pair>
#include <l4/cxx/std_alloc>

#include <l4/l4re_vfs/backend>
#include <l4/re/shared_cap>

#include <unistd.h>
#include <cstdarg>
#include <errno.h>
#include <sys/uio.h>

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
  Std_stream *s = new (m) Std_stream(L4Re::Env::env()->log());
  // make sure that we never delete the static io stream thing
  s->add_ref();
  set(0, cxx::ref_ptr(s)); // stdin
  set(1, cxx::ref_ptr(s)); // stdout
  set(2, cxx::ref_ptr(s)); // stderr
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
  int mremap(void *old, size_t old_sz, size_t new_sz, int flags,
             void **new_addr) noexcept override;
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

  l4_addr_t _anon_offset;
  L4Re::Shared_cap<L4Re::Dataspace> _anon_ds;

  int alloc_ds(unsigned long size, L4Re::Shared_cap<L4Re::Dataspace> *ds);
  int alloc_anon_mem(l4_umword_t size, L4Re::Shared_cap<L4Re::Dataspace> *ds,
                     l4_addr_t *offset);

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
  Cap<Dataspace> ds;
  Cap<Rm> r = Env::env()->rm();

  if (l4_addr_t(start) & (L4_PAGESIZE - 1))
    return -EINVAL;

  align_mmap_start_and_length(&start, &len);

  while (1)
    {
      DEBUG_LOG(debug_mmap, {
                outstring("DETACH: start = 0x");
                outhex32(l4_addr_t(start));
                outstring(" len = 0x");
                outhex32(len);
                outstring("\n");
      });
      err = r->detach(l4_addr_t(start), len, &ds, This_task);
      if (err < 0)
        return err;

      switch (err & Rm::Detach_result_mask)
        {
        case Rm::Split_ds:
          if (ds.is_valid())
            L4Re::virt_cap_alloc->take(ds);
          return 0;
        case Rm::Detached_ds:
          if (ds.is_valid())
            L4Re::virt_cap_alloc->release(ds);
          break;
        default:
          break;
        }

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
Vfs::alloc_ds(unsigned long size, L4Re::Shared_cap<L4Re::Dataspace> *ds)
{
  *ds = L4Re::make_shared_cap<L4Re::Dataspace>(L4Re::virt_cap_alloc);

  if (!ds->is_valid())
    return -ENOMEM;

  int err;
  if ((err = Vfs_config::allocator()->alloc(size, ds->get())) < 0)
    return err;

  DEBUG_LOG(debug_mmap, {
      outstring("ANON DS ALLOCATED: size=");
      outhex32(size);
      outstring("  cap = 0x");
      outhex32(ds->cap());
      outstring("\n");
      });

  return 0;
}

int
Vfs::alloc_anon_mem(l4_umword_t size, L4Re::Shared_cap<L4Re::Dataspace> *ds,
                    l4_addr_t *offset)
{
#ifdef USE_BIG_ANON_DS
  enum
  {
    ANON_MEM_DS_POOL_SIZE = 256UL << 20, // size of a pool dataspace used for anon memory
    ANON_MEM_MAX_SIZE     = 32UL << 20,  // chunk size that will be allocate a dataspace
  };
#else
  enum
  {
    ANON_MEM_DS_POOL_SIZE = 256UL << 20, // size of a pool dataspace used for anon memory
    ANON_MEM_MAX_SIZE     = 0UL << 20,   // chunk size that will be allocate a dataspace
  };
#endif

  if (size >= ANON_MEM_MAX_SIZE)
    {
      int err;
      if ((err = alloc_ds(size, ds)) < 0)
        return err;

      *offset = 0;

      if (!_early_oom)
        return err;

      return (*ds)->allocate(0, size);
    }

  if (!_anon_ds.is_valid() || _anon_offset + size >= ANON_MEM_DS_POOL_SIZE)
    {
      int err;
      if ((err = alloc_ds(ANON_MEM_DS_POOL_SIZE, ds)) < 0)
        return err;

      _anon_offset = 0;
      _anon_ds = *ds;
    }
  else
    *ds = _anon_ds;

  if (_early_oom)
    {
      if (int err = (*ds)->allocate(_anon_offset, size))
        return err;
    }

  *offset = _anon_offset;
  _anon_offset += size;
  return 0;
}

int
Vfs::mmap2(void *start, size_t len, int prot, int flags, int fd, off_t page4k_offset,
           void **resptr) L4_NOTHROW
{
  DEBUG_LOG(debug_mmap, {
            outstring("MMAP params: ");
            outstring("start = 0x");
            outhex32(l4_addr_t(start));
            outstring(", len = 0x");
            outhex32(len);
            outstring(", prot = 0x");
            outhex32(prot);
            outstring(", flags = 0x");
            outhex32(flags);
            outstring(", offset = 0x");
            outhex32(page4k_offset);
            outstring("\n");
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
                outstring("  MMAP reserved area: 0x");
                outhex32(area);
                outstring("  length= 0x");
                outhex32(len);
                outstring("\n");
      });

      return 0;
    }

  L4Re::Shared_cap<L4Re::Dataspace> ds;
  l4_addr_t anon_offset = 0;
  L4Re::Rm::Flags rm_flags(0);

  if (flags & (MAP_ANONYMOUS | MAP_PRIVATE))
    {
      rm_flags |= L4Re::Rm::F::Detach_free;

      int err = alloc_anon_mem(len, &ds, &anon_offset);
      if (err)
        return err;

      DEBUG_LOG(debug_mmap, {
                outstring("  USE ANON MEM: 0x");
                outhex32(ds.cap());
                outstring(" offs = 0x");
                outhex32(anon_offset);
                outstring("\n");
      });
    }

  if (!(flags & MAP_ANONYMOUS))
    {
      Ref_ptr<L4Re::Vfs::File> fi = fds.get(fd);
      if (!fi)
        return -EBADF;

      L4::Cap<L4Re::Dataspace> fds = fi->data_space();

      if (!fds.is_valid())
        return -EINVAL;

      if (len + offset > l4_round_page(fds->size()))
        return -EINVAL;

      if (flags & MAP_PRIVATE)
        {
          DEBUG_LOG(debug_mmap, outstring("COW\n"););
          int err = ds->copy_in(anon_offset, fds, offset, len);
          if (err == -L4_EINVAL)
            {
              L4::Cap<Rm> r = Env::env()->rm();
              Rm::Unique_region<char*> src;
              Rm::Unique_region<char*> dst;
              err = r->attach(&src, len,
                              L4Re::Rm::F::Search_addr | L4Re::Rm::F::R,
                              fds, offset);
              if (err < 0)
                return err;

              err = r->attach(&dst, len,
                              L4Re::Rm::F::Search_addr| L4Re::Rm::F::RW,
                              ds.get(), anon_offset);
              if (err < 0)
                return err;

              memcpy(dst.get(), src.get(), len);
            }
          else if (err)
            return err;

          offset = anon_offset;
        }
      else
        {
          L4Re::virt_cap_alloc->take(fds);
          ds = L4Re::Shared_cap<L4Re::Dataspace>(fds, L4Re::virt_cap_alloc);
        }
    }
  else
    offset = anon_offset;


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

  err = r->attach(&data, len, rm_flags,
                  L4::Ipc::make_cap(ds.get(), (prot & PROT_WRITE)
                                        ? L4_CAP_FPAGE_RW
                                        : L4_CAP_FPAGE_RO),
                  offset);

  DEBUG_LOG(debug_mmap, {
            outstring("  MAPPED: 0x");
            outhex32(ds.cap());
            outstring("  addr: 0x");
            outhex32(l4_addr_t(data));
            outstring("  bytes: 0x");
            outhex32(len);
            outstring("  offset: 0x");
            outhex32(offset);
            outstring("  err = ");
            outdec(err);
            outstring("\n");
  });


  if (overmap_area != L4_INVALID_ADDR)
    r->free_area(overmap_area);

  if (err < 0)
    return err;

  l4_assert (!(start && !data));

  // release ownership of the attached DS
  ds.release();
  *resptr = data;

  return 0;
}

namespace {
  class Auto_area
  {
  public:
    L4::Cap<L4Re::Rm> r;
    l4_addr_t a;

    explicit Auto_area(L4::Cap<L4Re::Rm> r, l4_addr_t a = L4_INVALID_ADDR)
    : r(r), a(a) {}

    int reserve(l4_addr_t _a, l4_size_t sz, L4Re::Rm::Flags flags)
    {
      free();
      a = _a;
      int e = r->reserve_area(&a, sz, flags);
      if (e)
        a = L4_INVALID_ADDR;
      return e;
    }

    void free()
    {
      if (is_valid())
        {
          r->free_area(a);
          a = L4_INVALID_ADDR;
        }
    }

    bool is_valid() const { return a != L4_INVALID_ADDR; }

    ~Auto_area() { free(); }
  };
}

int
Vfs::mremap(void *old_addr, size_t old_size, size_t new_size, int flags,
            void **new_addr) L4_NOTHROW
{
  using namespace L4Re;

  DEBUG_LOG(debug_mmap, {
            outstring("Mremap: addr = 0x");
            outhex32((l4_umword_t)old_addr);
            outstring(" old_size = 0x");
            outhex32(old_size);
            outstring("  new_size = 0x");
            outhex32(new_size);
            outstring("\n");
            });

  if (flags & MREMAP_FIXED && !(flags & MREMAP_MAYMOVE))
    return -EINVAL;

  l4_addr_t oa = l4_trunc_page(reinterpret_cast<l4_addr_t>(old_addr));
  if (oa != reinterpret_cast<l4_addr_t>(old_addr))
    return -EINVAL;

  bool const fixed = flags & MREMAP_FIXED;
  bool const maymove = flags & MREMAP_MAYMOVE;

  L4::Cap<Rm> r = Env::env()->rm();

  // sanitize input parameters to multiples of pages
  old_size = l4_round_page(old_size);
  new_size = l4_round_page(new_size);

  if (!fixed)
    {
      if (new_size < old_size)
        {
          *new_addr = old_addr;
          return munmap(reinterpret_cast<void*>(oa + new_size),
                        old_size - new_size);
        }

      if (new_size == old_size)
        {
          *new_addr = old_addr;
          return 0;
        }
    }

  Auto_area old_area(r);
  int err = old_area.reserve(oa, old_size, L4Re::Rm::Flags(0));
  if (err < 0)
    return -EINVAL;

  l4_addr_t pad_addr;
  Auto_area new_area(r);
  if (fixed)
    {
      l4_addr_t na = l4_trunc_page(reinterpret_cast<l4_addr_t>(*new_addr));
      if (na != reinterpret_cast<l4_addr_t>(*new_addr))
        return -EINVAL;

      // check if the current virtual memory area can be expanded
      int err = new_area.reserve(na, new_size, L4Re::Rm::Flags(0));
      if (err < 0)
        return err;

      pad_addr = na;
      // unmap all stuff and remap ours ....
    }
  else
    {
      l4_addr_t ta = oa + old_size;
      unsigned long ts = new_size - old_size;
      // check if the current virtual memory area can be expanded
      long err = new_area.reserve(ta, ts, L4Re::Rm::Flags(0));
      if (!maymove && err)
        return -ENOMEM;

      L4Re::Rm::Offset toffs;
      L4Re::Rm::Flags tflags;
      L4::Cap<L4Re::Dataspace> tds;

      err = r->find(&ta, &ts, &toffs, &tflags, &tds);

      // there is enough space to expand the mapping in place
      if (err == -ENOENT || (err == 0 && (tflags & Rm::F::In_area)))
        {
          old_area.free(); // pad at the original address
          pad_addr = oa + old_size;
          *new_addr = old_addr;
        }
      else if (!maymove)
        return -ENOMEM;
      else
        {
          // search for a new area to remap
          err = new_area.reserve(0, new_size, Rm::F::Search_addr);
          if (err < 0)
            return -ENOMEM;

          pad_addr = new_area.a + old_size;
          *new_addr = reinterpret_cast<void *>(new_area.a);
        }
    }

  if (old_area.is_valid())
    {
      l4_addr_t a = old_area.a;
      unsigned long s = old_size;
      L4Re::Rm::Offset o;
      L4Re::Rm::Flags f;
      L4::Cap<L4Re::Dataspace> ds;

      for (; r->find(&a, &s, &o, &f, &ds) >= 0 && (!(f & Rm::F::In_area));)
        {
          if (a < old_area.a)
            {
              auto d = old_area.a - a;
              a = old_area.a;
              s -= d;
              o += d;
            }

          if (a + s > old_area.a + old_size)
            s = old_area.a + old_size - a;

          l4_addr_t x = a - old_area.a + new_area.a;

          int err = r->attach(&x, s, Rm::F::In_area | f,
                              L4::Ipc::make_cap(ds, f.cap_rights()),
                              o);
          if (err < 0)
            return err;

          // cout the new attached ds reference
          L4Re::virt_cap_alloc->take(ds);

          err = r->detach(a, s, &ds, This_task,
                          Rm::Detach_exact |  Rm::Detach_keep);
          if (err < 0)
            return err;

          switch (err & Rm::Detach_result_mask)
            {
            case Rm::Split_ds:
              // add a reference as we split up a mapping
              if (ds.is_valid())
                L4Re::virt_cap_alloc->take(ds);
              break;
            case Rm::Detached_ds:
              if (ds.is_valid())
                L4Re::virt_cap_alloc->release(ds);
              break;
            default:
              break;
            }
        }
      old_area.free();
    }

  if (old_size < new_size)
    {
      l4_addr_t const pad_sz = new_size - old_size;
      l4_addr_t toffs;
      L4Re::Shared_cap<L4Re::Dataspace> tds;
      int err = alloc_anon_mem(pad_sz, &tds, &toffs);
      if (err)
        return err;

      // FIXME: must get the protection rights from the old
      // mapping and use the same here, for now just use RWX
      err = r->attach(&pad_addr, pad_sz,
                      Rm::F::In_area | Rm::F::Detach_free | Rm::F::RWX,
                      L4::Ipc::make_cap_rw(tds.get()), toffs);
      if (err < 0)
        return err;

      // release ownership of tds, the region map is now the new owner
      tds.release();
    }

  return 0;
}

int
Vfs::mprotect(const void * /* a */, size_t /* sz */, int prot) L4_NOTHROW
{
  return (prot & PROT_WRITE) ? -1 : 0;
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
