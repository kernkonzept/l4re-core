/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "ns_fs.h"

#include <l4/re/dataspace>
#include <l4/re/util/env_ns>
#include <l4/re/unique_cap>
#include <dirent.h>

namespace L4Re { namespace Core {

static
Ref_ptr<L4Re::Vfs::File>
cap_to_vfs_object(L4::Cap<void> o, int *err)
{
  L4::Cap<L4::Meta> m = L4::cap_reinterpret_cast<L4::Meta>(o);
  long proto = 0;
  char name_buf[256];
  L4::Ipc::String<char> name(sizeof(name_buf), name_buf);
  int r = l4_error(m->interface(0, &proto, &name));
  *err = -ENOPROTOOPT;
  if (r < 0)
    // could not get type of object so bail out
    return Ref_ptr<L4Re::Vfs::File>();

  *err = -EPROTO;
  Ref_ptr<L4Re::Vfs::File_factory> factory;

  if (proto != 0)
    factory = L4Re::Vfs::vfs_ops->get_file_factory(proto);

  if (!factory)
    factory = L4Re::Vfs::vfs_ops->get_file_factory(name.data);

  if (!factory)
    return Ref_ptr<L4Re::Vfs::File>();

  *err = -ENOMEM;
  return factory->create(o);
}


int
Ns_dir::get_ds(const char *path, L4Re::Unique_cap<L4Re::Dataspace> *ds) noexcept
{
  auto file = L4Re::make_unique_cap<L4Re::Dataspace>(L4Re::virt_cap_alloc);

  if (!file.is_valid())
    return -ENOMEM;

  int err = _ns->query(path, file.get());

  if (err < 0)
    return -ENOENT;

  *ds = cxx::move(file);
  return err;
}

int
Ns_dir::get_entry(const char *path, int /*flags*/, mode_t /*mode*/,
                  Ref_ptr<L4Re::Vfs::File> *f) noexcept
{
  if (!*path)
    {
      *f = cxx::ref_ptr(this);
      return 0;
    }

  L4Re::Unique_cap<Dataspace> file;
  int err = get_ds(path, &file);

  if (err < 0)
    return -ENOENT;

  cxx::Ref_ptr<L4Re::Vfs::File> fi = cap_to_vfs_object(file.get(), &err);
  if (!fi)
    return err;

  file.release();
  *f = cxx::move(fi);
  return 0;
}

int
Ns_dir::faccessat(const char *path, int mode, int /*flags*/) noexcept
{
  auto tmpcap = L4Re::make_unique_cap<void>(L4Re::virt_cap_alloc);

  if (!tmpcap.is_valid())
    return -ENOMEM;

  if (_ns->query(path, tmpcap.get()))
    return -ENOENT;

  if (mode & W_OK)
    return -EACCES;

  return 0;
}

int
Ns_dir::fstat(struct stat64 *b) const noexcept
{
  b->st_dev = 1;
  b->st_ino = 1;
  b->st_mode = S_IRWXU | S_IFDIR;
  b->st_nlink = 0;
  b->st_uid = 0;
  b->st_gid = 0;
  b->st_rdev = 0;
  b->st_size = 0;
  b->st_blksize = 0;
  b->st_blocks = 0;
  b->st_atime = 0;
  b->st_mtime = 0;
  b->st_ctime = 0;
  return 0;
}

ssize_t
Ns_dir::getdents(char *buf, size_t dest_sz) noexcept
{
  struct dirent64 *dest = reinterpret_cast<struct dirent64 *>(buf);
  ssize_t ret = 0;
  l4_addr_t infoaddr;
  size_t infosz;

  L4Re::Unique_cap<Dataspace> dirinfofile;
  int err = get_ds(".dirinfo", &dirinfofile);
  if (err)
    return 0;

  infosz = dirinfofile->size();
  if (infosz <= 0)
    return 0;

  infoaddr = L4_PAGESIZE;
  err = L4Re::Env::env()->rm()->attach(&infoaddr, infosz,
                                       Rm::F::Search_addr | Rm::F::R,
                                       dirinfofile.get(), 0);
  if (err < 0)
    return 0;

  char *p   = reinterpret_cast<char *>(infoaddr) + _current_dir_pos;
  char *end = reinterpret_cast<char *>(infoaddr) + infosz;

  char *current_dirinfo_entry = p;
  while (dest && p < end)
    {
      // parse lines of dirinfofile
      long len = 0;
      for (; p < end && *p >= '0' && *p <= '9'; ++p)
        {
          len *= 10;
          len += *p - '0';
        }

      if (len == 0)
        break;

      if (p == end)
        break;

      if (*p != ':')
        break;
      p++; // skip colon

      if (p + len >= end)
        break;

      unsigned l = len + 1;
      if (l > sizeof(dest->d_name))
        l = sizeof(dest->d_name);

      unsigned n = offsetof (struct dirent64, d_name) + l;
      n = (n + sizeof(long) - 1) & ~(sizeof(long) - 1);

      if (n > dest_sz)
        break;

      dest->d_ino = 1;
      dest->d_off = 0;
      memcpy(dest->d_name, p, l - 1);
      dest->d_name[l - 1] = 0;
      dest->d_reclen = n;
      dest->d_type   = DT_UNKNOWN;
      ret += n;
      dest_sz -= n;

      // next entry
      dest = reinterpret_cast<struct dirent64 *>
               (reinterpret_cast<unsigned long>(dest) + n);

      // next infodirfile line
      p += len;
      while (p < end && *p && (*p == '\n' || *p == '\r'))
        p++;

      current_dirinfo_entry = p;
    }

  _current_dir_pos = current_dirinfo_entry - reinterpret_cast<char *>(infoaddr);

  if (!ret) // hack since we should only reset this at open times
    _current_dir_pos = 0;

  L4Re::Env::env()->rm()->detach(infoaddr, 0);

  return ret;
}

int
Env_dir::get_ds(const char *path, L4Re::Unique_cap<L4Re::Dataspace> *ds) noexcept
{
  Vfs::Path p(path);
  Vfs::Path first = p.strip_first();

  if (first.empty())
    return -ENOENT;

  L4::Cap<L4Re::Namespace>
    c = _env->get_cap<L4Re::Namespace>(first.path(), first.length());

  if (!c.is_valid())
    return -ENOENT;

  if (p.empty())
    {
      *ds = L4Re::Unique_cap<L4Re::Dataspace>(L4::cap_reinterpret_cast<L4Re::Dataspace>(c));
      return 0;
    }

  auto file = L4Re::make_unique_cap<L4Re::Dataspace>(L4Re::virt_cap_alloc);

  if (!file.is_valid())
    return -ENOMEM;

  int err = c->query(p.path(), p.length(), file.get());

  if (err < 0)
    return -ENOENT;

  *ds = cxx::move(file);
  return err;
}

int
Env_dir::get_entry(const char *path, int /*flags*/, mode_t /*mode*/,
                   Ref_ptr<L4Re::Vfs::File> *f) noexcept
{
  if (!*path)
    {
      *f = cxx::ref_ptr(this);
      return 0;
    }

  L4Re::Unique_cap<Dataspace> file;
  int err = get_ds(path, &file);

  if (err < 0)
    return -ENOENT;

  cxx::Ref_ptr<L4Re::Vfs::File> fi = cap_to_vfs_object(file.get(), &err);
  if (!fi)
    return err;

  file.release();
  *f = cxx::move(fi);
  return 0;
}

int
Env_dir::faccessat(const char *path, int mode, int /*flags*/) noexcept
{
  Vfs::Path p(path);
  Vfs::Path first = p.strip_first();

  if (first.empty())
    return -ENOENT;

  L4::Cap<L4Re::Namespace>
    c = _env->get_cap<L4Re::Namespace>(first.path(), first.length());

  if (!c.is_valid())
    return -ENOENT;

  if (p.empty())
    {
      if (mode & W_OK)
	return -EACCES;

      return 0;
    }

  auto tmpcap = L4Re::make_unique_cap<void>(L4Re::virt_cap_alloc);

  if (!tmpcap.is_valid())
    return -ENOMEM;

  if (c->query(p.path(), p.length(), tmpcap.get()))
    return -ENOENT;

  if (mode & W_OK)
    return -EACCES;

  return 0;
}

bool
Env_dir::check_type(Env::Cap_entry const *e, long protocol) noexcept
{
  L4::Cap<L4::Meta> m(e->cap);
  return m->supports(protocol).label();
}

int
Env_dir::fstat(struct stat64 *b) const noexcept
{
  b->st_dev = 1;
  b->st_ino = 1;
  b->st_mode = S_IRWXU | S_IFDIR;
  b->st_nlink = 0;
  b->st_uid = 0;
  b->st_gid = 0;
  b->st_rdev = 0;
  b->st_size = 0;
  b->st_blksize = 0;
  b->st_blocks = 0;
  b->st_atime = 0;
  b->st_mtime = 0;
  b->st_ctime = 0;
  return 0;
}

ssize_t
Env_dir::getdents(char *buf, size_t sz) noexcept
{
  struct dirent64 *d = reinterpret_cast<struct dirent64 *>(buf);
  ssize_t ret = 0;

  while (d
         && _current_cap_entry
         && _current_cap_entry->flags != ~0UL)
    {
      unsigned l = strlen(_current_cap_entry->name) + 1;
      if (l > sizeof(d->d_name))
        l = sizeof(d->d_name);

      unsigned n = offsetof (struct dirent64, d_name) + l;
      n = (n + sizeof(long) - 1) & ~(sizeof(long) - 1);

      if (n <= sz)
        {
          d->d_ino = 1;
          d->d_off = 0;
          memcpy(d->d_name, _current_cap_entry->name, l);
          d->d_name[l - 1] = 0;
          d->d_reclen = n;
          if (check_type(_current_cap_entry, L4Re::Namespace::Protocol))
            d->d_type = DT_DIR;
          else if (check_type(_current_cap_entry, L4Re::Dataspace::Protocol))
            d->d_type = DT_REG;
          else
            d->d_type = DT_UNKNOWN;
          ret += n;
          sz  -= n;
          d    = reinterpret_cast<struct dirent64 *>
                   (reinterpret_cast<unsigned long>(d) + n);
          _current_cap_entry++;
        }
      else
        return ret;
    }

  // bit of a hack because we should only (re)set this when opening the dir
  if (!ret)
    _current_cap_entry = _env->initial_caps();

  return ret;
}

}}
