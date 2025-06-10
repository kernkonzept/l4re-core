/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/l4re_vfs/backend>
#include <l4/sys/capability>
#include <l4/re/namespace>
#include <l4/re/unique_cap>

namespace L4Re { namespace Core {

using cxx::Ref_ptr;

class Env_dir : public L4Re::Vfs::Be_file
{
public:
  explicit Env_dir(L4Re::Env const *env)
  : _env(env), _current_cap_entry(env->initial_caps())
  {}

  ssize_t readv(const struct iovec*, int) noexcept override { return -EISDIR; }
  ssize_t writev(const struct iovec*, int) noexcept override { return -EISDIR; }
  ssize_t preadv(const struct iovec*, int, off64_t) noexcept override { return -EISDIR; }
  ssize_t pwritev(const struct iovec*, int, off64_t) noexcept override { return -EISDIR; }
  int fstat(struct stat64 *) const noexcept override;
  int faccessat(const char *path, int mode, int flags) noexcept override;
  int get_entry(const char *path, int flags, mode_t mode,
                Ref_ptr<L4Re::Vfs::File> *) noexcept override;
  ssize_t getdents(char *, size_t) noexcept override;

  ~Env_dir() noexcept {}

private:
  int get_ds(const char *path, L4Re::Unique_cap<L4Re::Dataspace> *ds) noexcept;
  bool check_type(Env::Cap_entry const *e, long protocol) noexcept;

  L4Re::Env const *_env;
  Env::Cap_entry const *_current_cap_entry;
};

class Ns_dir : public L4Re::Vfs::Be_file
{
public:
  explicit Ns_dir(L4::Cap<L4Re::Namespace> ns)
  : _ns(ns), _current_dir_pos(0)
  {}

  ssize_t readv(const struct iovec*, int) noexcept override { return -EISDIR; }
  ssize_t writev(const struct iovec*, int) noexcept override { return -EISDIR; }
  ssize_t preadv(const struct iovec*, int, off64_t) noexcept override { return -EISDIR; }
  ssize_t pwritev(const struct iovec*, int, off64_t) noexcept override { return -EISDIR; }
  int fstat(struct stat64 *) const noexcept override;
  int faccessat(const char *path, int mode, int flags) noexcept override;
  int get_entry(const char *path, int flags, mode_t mode,
                Ref_ptr<L4Re::Vfs::File> *) noexcept override;
  ssize_t getdents(char *, size_t) noexcept override;

  ~Ns_dir() noexcept {}

private:
  int get_ds(const char *path, L4Re::Unique_cap<L4Re::Dataspace> *ds) noexcept;

  L4::Cap<L4Re::Namespace> _ns;
  size_t _current_dir_pos;
};

}}
