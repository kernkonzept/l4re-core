/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
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

  ssize_t readv(const struct iovec*, int) throw() { return -EISDIR; }
  ssize_t writev(const struct iovec*, int) throw() { return -EISDIR; }
  ssize_t preadv(const struct iovec*, int, off64_t) throw() { return -EISDIR; }
  ssize_t pwritev(const struct iovec*, int, off64_t) throw() { return -EISDIR; }
  int fstat64(struct stat64 *) const throw();
  int faccessat(const char *path, int mode, int flags) throw();
  int get_entry(const char *path, int flags, mode_t mode,
                Ref_ptr<L4Re::Vfs::File> *) throw();
  ssize_t getdents(char *, size_t) throw();

  ~Env_dir() throw() {}

private:
  int get_ds(const char *path, L4Re::Unique_cap<L4Re::Dataspace> *ds) throw();
  bool check_type(Env::Cap_entry const *e, long protocol) throw();

  L4Re::Env const *_env;
  Env::Cap_entry const *_current_cap_entry;
};

class Ns_dir : public L4Re::Vfs::Be_file
{
public:
  explicit Ns_dir(L4::Cap<L4Re::Namespace> ns)
  : _ns(ns), _current_dir_pos(0)
  {}

  ssize_t readv(const struct iovec*, int) throw() { return -EISDIR; }
  ssize_t writev(const struct iovec*, int) throw() { return -EISDIR; }
  ssize_t preadv(const struct iovec*, int, off64_t) throw() { return -EISDIR; }
  ssize_t pwritev(const struct iovec*, int, off64_t) throw() { return -EISDIR; }
  int fstat64(struct stat64 *) const throw();
  int faccessat(const char *path, int mode, int flags) throw();
  int get_entry(const char *path, int flags, mode_t mode,
                Ref_ptr<L4Re::Vfs::File> *) throw();
  ssize_t getdents(char *, size_t) throw();

  ~Ns_dir() throw() {}

private:
  int get_ds(const char *path, L4Re::Unique_cap<L4Re::Dataspace> *ds) throw();

  L4::Cap<L4Re::Namespace> _ns;
  size_t _current_dir_pos;
};

}}
