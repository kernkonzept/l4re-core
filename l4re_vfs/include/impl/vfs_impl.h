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
  Fd_store() throw();
};

// for internal Vcon_streams we want to have a placement new operator, so
// inherit and add one
class Std_stream : public L4Re::Core::Vcon_stream
{
public:
  Std_stream(L4::Cap<L4::Vcon> c) : L4Re::Core::Vcon_stream(c) {}
};

Fd_store::Fd_store() throw()
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

  int alloc_fd(Ref_ptr<L4Re::Vfs::File> const &f) throw();
  Ref_ptr<L4Re::Vfs::File> free_fd(int fd) throw();
  Ref_ptr<L4Re::Vfs::File> get_root() throw();
  Ref_ptr<L4Re::Vfs::File> get_cwd() throw();
  void set_cwd(Ref_ptr<L4Re::Vfs::File> const &dir) throw();
  Ref_ptr<L4Re::Vfs::File> get_file(int fd) throw();
  Ref_ptr<L4Re::Vfs::File> set_fd(int fd, Ref_ptr<L4Re::Vfs::File> const &f = Ref_ptr<>::Nil) throw();

  int mmap2(void *start, size_t len, int prot, int flags, int fd,
            off_t offset, void **ptr) throw();

  int munmap(void *start, size_t len) throw();
  int mremap(void *old, size_t old_sz, size_t new_sz, int flags,
             void **new_addr) throw();
  int mprotect(const void *a, size_t sz, int prot) throw();
  int msync(void *addr, size_t len, int flags) throw();
  int madvise(void *addr, size_t len, int advice) throw();

  int register_file_system(L4Re::Vfs::File_system *f) throw();
  int unregister_file_system(L4Re::Vfs::File_system *f) throw();
  L4Re::Vfs::File_system *get_file_system(char const *fstype) throw();

  int register_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) throw();
  int unregister_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) throw();
  Ref_ptr<L4Re::Vfs::File_factory> get_file_factory(int proto) throw();
  Ref_ptr<L4Re::Vfs::File_factory> get_file_factory(char const *proto_name) throw();
  int mount(char const *path, cxx::Ref_ptr<L4Re::Vfs::File> const &dir) throw();

  void operator delete (void *) {}

  void *malloc(size_t size) noexcept { return Vfs_config::malloc(size); }
  void free(void *m) noexcept { Vfs_config::free(m); }

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
};

static inline bool strequal(char const *a, char const *b)
{
  for (;*a && *a == *b; ++a, ++b)
    ;
  return *a == *b;
}

int
Vfs::register_file_system(L4Re::Vfs::File_system *f) throw()
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
Vfs::unregister_file_system(L4Re::Vfs::File_system *f) throw()
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
Vfs::get_file_system(char const *fstype) throw()
{
  bool try_dynamic = true;
  for (;;)
    {
      using L4Re::Vfs::File_system;
      for (File_system *c = _fs_registry; c; c = c->next())
	if (strequal(c->type(), fstype))
	  return c;

      if (!try_dynamic)
	return 0;

      // try to load a file system module dynamically
      int res = Vfs_config::load_module(fstype);

      if (res < 0)
	return 0;

      try_dynamic = false;
    }
}

int
Vfs::register_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) throw()
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
Vfs::unregister_file_factory(cxx::Ref_ptr<L4Re::Vfs::File_factory> f) throw()
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
Vfs::get_file_factory(int proto) throw()
{
  for (auto p: _file_factories)
    if (p->f->proto() == proto)
      return p->f;

  return Ref_ptr<L4Re::Vfs::File_factory>();
}

Ref_ptr<L4Re::Vfs::File_factory>
Vfs::get_file_factory(char const *proto_name) throw()
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
Vfs::alloc_fd(Ref_ptr<L4Re::Vfs::File> const &f) throw()
{
  int fd = fds.alloc();
  if (fd < 0)
    return -EMFILE;

  if (f)
    fds.set(fd, f);

  return fd;
}

Ref_ptr<L4Re::Vfs::File>
Vfs::free_fd(int fd) throw()
{
  Ref_ptr<L4Re::Vfs::File> f = fds.get(fd);

  if (!f)
    return Ref_ptr<>::Nil;

  fds.free(fd);
  return f;
}


Ref_ptr<L4Re::Vfs::File>
Vfs::get_root() throw()
{
  return cxx::ref_ptr(&_root);
}

Ref_ptr<L4Re::Vfs::File>
Vfs::get_cwd() throw()
{
  return _cwd;
}

void
Vfs::set_cwd(Ref_ptr<L4Re::Vfs::File> const &dir) throw()
{
  // FIXME: check for is dir
  if (dir)
    _cwd = dir;
}

Ref_ptr<L4Re::Vfs::File>
Vfs::get_file(int fd) throw()
{
  return fds.get(fd);
}

Ref_ptr<L4Re::Vfs::File>
Vfs::set_fd(int fd, Ref_ptr<L4Re::Vfs::File> const &f) throw()
{
  Ref_ptr<L4Re::Vfs::File> old = fds.get(fd);
  fds.set(fd, f);
  return old;
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


int
Vfs::munmap(void *start, size_t len) L4_NOTHROW
{
  using namespace L4;
  using namespace L4Re;

  int err;
  Cap<Dataspace> ds;
  Cap<Rm> r = Env::env()->rm();

  while (1)
    {
      DEBUG_LOG(debug_mmap, {
	  outstring("DETACH: ");
	  outhex32(l4_addr_t(start));
	  outstring(" ");
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
      outstring("  cap=");
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
Vfs::mmap2(void *start, size_t len, int prot, int flags, int fd, off_t _offset,
           void **resptr) L4_NOTHROW
{
  using namespace L4Re;
  off64_t offset = l4_trunc_page(_offset << 12);

  start = (void*)l4_trunc_page(l4_addr_t(start));
  len   = l4_round_page(len);
  l4_umword_t size = (len + L4_PAGESIZE-1) & ~(L4_PAGESIZE-1);

  // special code to just reserve an area of the virtual address space
  if (flags & 0x1000000)
    {
      int err;
      L4::Cap<Rm> r = Env::env()->rm();
      l4_addr_t area = (l4_addr_t)start;
      err = r->reserve_area(&area, size, L4Re::Rm::Search_addr);
      if (err < 0)
	return err;
      *resptr = (void*)area;
      DEBUG_LOG(debug_mmap, {
	  outstring("MMAP reserved area: ");
	  outhex32(area);
	  outstring("  size=");
	  outhex32(size);
	  outstring("\n");
      });
      return 0;
    }

  L4Re::Shared_cap<L4Re::Dataspace> ds;
  l4_addr_t anon_offset = 0;
  unsigned rm_flags = 0;

  if (flags & (MAP_ANONYMOUS | MAP_PRIVATE))
    {
      rm_flags |= L4Re::Rm::Detach_free;

      int err = alloc_anon_mem(size, &ds, &anon_offset);
      if (err)
	return err;

      DEBUG_LOG(debug_mmap, {
	  outstring("USE ANON MEM: ");
	  outhex32(ds.cap());
	  outstring(" offs=");
	  outhex32(anon_offset);
	  outstring("\n");
      });
    }

  if (!(flags & MAP_ANONYMOUS))
    {
      Ref_ptr<L4Re::Vfs::File> fi = fds.get(fd);
      if (!fi)
	{
	  return -EBADF;
	}

      L4::Cap<L4Re::Dataspace> fds = fi->data_space();

      if (!fds.is_valid())
	{
	  return -EINVAL;
	}

      if (size + offset > l4_round_page(fds->size()))
	{
	  return -EINVAL;
	}

      if (flags & MAP_PRIVATE)
	{
	  DEBUG_LOG(debug_mmap, outstring("COW\n"););
	  ds->copy_in(anon_offset, fds, l4_trunc_page(offset), l4_round_page(size));
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
    start = (void*)L4_PAGESIZE;

  int err;
  char *data = (char *)start;
  L4::Cap<Rm> r = Env::env()->rm();
  l4_addr_t overmap_area = L4_INVALID_ADDR;

  if (flags & MAP_FIXED)
    {
      overmap_area = l4_addr_t(start);

      err = r->reserve_area(&overmap_area, size);
      if (err < 0)
	overmap_area = L4_INVALID_ADDR;

      rm_flags |= Rm::In_area;

      err = munmap(start, len);
      if (err && err != -ENOENT)
	return err;
    }

  if (!(flags & MAP_FIXED))  rm_flags |= Rm::Search_addr;
  if (!(prot & PROT_WRITE))  rm_flags |= Rm::Read_only;

  err = r->attach(&data, size, rm_flags,
                  L4::Ipc::make_cap(ds.get(), (prot & PROT_WRITE)
                                        ? L4_CAP_FPAGE_RW
                                        : L4_CAP_FPAGE_RO),
                  offset);

  DEBUG_LOG(debug_mmap, {
      outstring("  MAPPED: ");
      outhex32(ds.cap());
      outstring("  addr: ");
      outhex32(l4_addr_t(data));
      outstring("  bytes: ");
      outhex32(size);
      outstring("  offset: ");
      outhex32(offset);
      outstring("  err=");
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

    int reserve(l4_addr_t _a, l4_size_t sz, unsigned flags)
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
            outstring("Mremap: addr=");
            outhex32((l4_umword_t)old_addr);
            outstring(" old_size=");
            outhex32(old_size);
            outstring("  new_size=");
            outhex32(new_size);
            outstring("\n");
            });

  if (flags & MREMAP_FIXED && !(flags & MREMAP_MAYMOVE))
    return -EINVAL;

  l4_addr_t oa = l4_trunc_page((l4_addr_t)old_addr);
  if (oa != (l4_addr_t)old_addr)
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
          return munmap((void*)(oa + new_size), old_size - new_size);
        }

      if (new_size == old_size)
        {
          *new_addr = old_addr;
          return 0;
        }
    }

  Auto_area old_area(r);
  int err = old_area.reserve(oa, old_size, 0);
  if (err < 0)
    return -EINVAL;

  l4_addr_t pad_addr;
  Auto_area new_area(r);
  if (fixed)
    {
      l4_addr_t na = l4_trunc_page((l4_addr_t)*new_addr);
      if (na != (l4_addr_t)*new_addr)
        return -EINVAL;

      // check if the current virtual memory area can be expanded
      int err = new_area.reserve(na, new_size, 0);
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
      int err = new_area.reserve(ta, ts, 0);
      if (!maymove && err)
        return -ENOMEM;

      l4_addr_t toffs;
      unsigned tflags;
      L4::Cap<L4Re::Dataspace> tds;

      err = r->find(&ta, &ts, &toffs, &tflags, &tds);

      // there is enough space to expand the mapping in place
      if (err == -ENOENT || (err == 0 && (tflags & Rm::In_area)))
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
          err = new_area.reserve(0, new_size, Rm::Search_addr);
          if (err < 0)
            return -ENOMEM;

          pad_addr = new_area.a + old_size;
          *new_addr = (void *)new_area.a;
        }
    }

  if (old_area.is_valid())
    {
      l4_addr_t a = old_area.a;
      unsigned long s = old_size;
      l4_addr_t o;
      unsigned f;
      L4::Cap<L4Re::Dataspace> ds;

      for (; r->find(&a, &s, &o, &f, &ds) >= 0 && (!(f & Rm::In_area));)
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

          int err = r->attach(&x, s, Rm::In_area | f,
                              L4::Ipc::make_cap(ds, (f & Rm::Read_only)
                                                    ? L4_CAP_FPAGE_RO
                                                    : L4_CAP_FPAGE_RW),
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

      err = r->attach(&pad_addr, pad_sz, Rm::In_area | Rm::Detach_free,
                      L4::Ipc::make_cap_rw(tds.get()), toffs);
      if (err < 0)
        return err;

      // release ownership of tds, the region map is now the new owner
      tds.release();
    }

  return 0;
}

int
Vfs::mprotect(const void *a, size_t sz, int prot) L4_NOTHROW
{
  (void)a;
  (void)sz;
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
Vfs::mount(char const *path, cxx::Ref_ptr<L4Re::Vfs::File> const &dir) throw()
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

      cxx::Ref_ptr<Mount_tree> nt(new Real_mount_tree(name));
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

