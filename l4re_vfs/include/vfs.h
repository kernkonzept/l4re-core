/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>

#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <utime.h>
#include <errno.h>

#ifndef AT_FDCWD
# define AT_FDCWD -100
#endif

#ifdef __cplusplus

#include <l4/sys/capability>
#include <l4/re/cap_alloc>
#include <l4/re/dataspace>
#include <l4/cxx/pair>
#include <l4/cxx/ref_ptr>

namespace L4Re {
/**
 * \brief Virtual file system for interfaces in POSIX libc.
 */
namespace Vfs {

class Mount_tree;
class File;

/**
 * \brief The common interface for an open POSIX file.
 *
 * This interface is common to all kinds of open files, independent of
 * the file type (e.g., directory, regular file etc.).  However, in
 * the L4Re::Vfs the interface File is used for every real object.
 *
 * \see L4Re::Vfs::File for more information.
 */
class Generic_file
{
public:
  /**
   * \brief Type of I/O operation/condition a file can indicate readiness.
   *
   * As defined by select() and similar functions.
   */
  enum Ready_type : unsigned
  {
    Read = 0,
    Write,
    Exception
  };

  virtual ~Generic_file() noexcept = 0;

  /**
   * \brief Unlock all locks on the file.
   * \note All locks means all locks independent of which file
   *       the locks were taken by.
   *
   * This method is called by the POSIX close implementation to
   * get the POSIX semantics of releasing all locks taken by this
   * application on a close for any fd referencing the real file.
   *
   * \return 0 on success, or <0 on error.
   */
  virtual int unlock_all_locks() noexcept = 0;

  /**
   * \brief Get status information for the file.
   *
   * This is the backend for POSIX fstat, stat, fstat64 and friends.
   *
   * \param[out] buf This buffer is filled with the status information.
   * \return 0 on success, or <0 on error.
   */
  virtual int fstat64(struct stat64 *buf) const noexcept = 0;

  /**
   * \brief Change POSIX access rights on the file.
   *
   * Backend for POSIX chmod and fchmod.
   */
  virtual int fchmod(mode_t) noexcept = 0;

  /**
   * \brief Get file status flags (fcntl F_GETFL).
   *
   * This function is used by the fcntl implementation for the F_GETFL
   * command.
   *
   * \return flags such as `O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_DIRECT`,
   *         `O_ASYNC`, `O_NOATIME`, `O_NONBLOCK`, or <0 on error.
   */
  virtual int get_status_flags() const noexcept = 0;

  /**
   * \brief Set file status flags (fcntl F_SETFL).
   *
   * This function is used by the fcntl implementation for the F_SETFL
   * command.
   *
   * \param flags The file status flags to set. This must be a combination of
   *              `O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_APPEND`, `O_ASYNC`,
   *              `O_DIRECT`, `O_NOATIME`, `O_NONBLOCK`.
   *
   * \note Creation flags such as `O_CREAT`, `O_EXCL`, `O_NOCTTY`, `O_TRUNC`
   *       are ignored.
   *
   * \return 0 on success, or <0 on error.
   */
  virtual int set_status_flags(long flags) noexcept = 0;

  virtual int utime(const struct utimbuf *) noexcept = 0;
  virtual int utimes(const struct timeval [2]) noexcept = 0;
  virtual ssize_t readlink(char *, size_t) = 0;

  /**
   * \brief Check whether the file is ready for an I/O operation/condition.
   *
   * This method is used by the implementation of select() and similar
   * functions.
   *
   * \param rt  Type of the I/O operation/condition to be ready, as defined
   *            by the select() and similar functions (#Read, #Write,
   *            #Exception).
   *
   * \retval true   The file is ready for the given type of I/O
   *                operation/condition.
   * \retval false  The file is not ready for the given type of I/O
   *                operation/condition.
   */
  virtual bool check_ready(Ready_type rt) noexcept = 0;
};

inline
Generic_file::~Generic_file() noexcept
{}

/**
 * \brief Interface for a POSIX file that is a directory.
 *
 * This interface provides functionality for directory files
 * in the L4Re::Vfs. However, real objects always use the
 * combined L4Re::Vfs::File interface.
 */
class Directory
{
public:
  virtual ~Directory() noexcept = 0;

  /**
   * \brief Check access permissions on the given file.
   *
   * Backend function for POSIX access and faccessat functions.
   *
   * \param path The path relative to this directory.
   *             Note: \a path is relative to this directory and
   *             may contain subdirectories.
   * \param mode The access mode to check.
   * \param flags The flags as in POSIX faccessat (AT_EACCESS,
   *              AT_SYMLINK_NOFOLLOW).
   * \return 0 on success, or <0 on error.
   */
  virtual int faccessat(const char *path, int mode, int flags) noexcept = 0;

  /**
   * \brief Create a new subdirectory.
   *
   * Backend for POSIX mkdir and mkdirat function calls.
   *
   * \param path The name of the subdirectory to create.
   *             Note: \a path is relative to this directory and
   *             may contain subdirectories.
   * \param mode The file mode to use for the new directory.
   * \return 0 on success, or <0 on error. -ENOTDIR if this or some component
   *         in path is not a directory.
   */
  virtual int mkdir(const char *path, mode_t mode) noexcept = 0;

  /**
   * \brief Unlink the given file from that directory.
   *
   * Backend for the POSIX unlink and unlinkat functions.
   *
   * \param path The name of the file to unlink. Note: \a path
   *             is relative to this directory and may
   *             contain subdirectories.
   * \return 0 on success, or <0 on error.
   */
  virtual int unlink(const char *path) noexcept = 0;

  /**
   * \brief Rename the given file.
   *
   * Backend for the POSIX rename, renameat functions.
   *
   * \param src_path The old name of the file to rename.
   *                 Note: \a src_path is relative to this
   *                 directory and may contain subdirectories.
   * \param dst_path The new name for the file.
   *                 Note: \a dst_path is relative to this
   *                 directory and may contain subdirectories.
   * \return 0 on success, or <0 on error.
   */
  virtual int rename(const char *src_path, const char *dst_path) noexcept = 0;

  /**
   * \brief Create a hard link (second name) for the given file.
   *
   * Backend for the POSIX link and linkat functions.
   *
   * \param src_path The old name of the file.
   *                 Note: \a src_path is relative to this
   *                 directory and may contain subdirectories.
   * \param dst_path The new (second) name for the file.
   *                 Note: \a dst_path is relative to this
   *                 directory and may contain subdirectories.
   * \return 0 on success, or <0 on error.
   */
  virtual int link(const char *src_path, const char *dst_path) noexcept = 0;

  /**
   * \brief Create a symbolic link for the given file.
   *
   * Backend for the POSIX symlink and symlinkat functions.
   *
   * \param src_path The old name of the file.
   *                 Note: \a src_path shall be an absolute path.
   * \param dst_path The name for symlink.
   *                 Note: \a dst_path is relative to this
   *                 directory and may contain subdirectories.
   * \return 0 on success, or <0 on error.
   */
  virtual int symlink(const char *src_path, const char *dst_path) noexcept = 0;

  /**
   * \brief Delete an empty directory.
   *
   * Backend for POSIX rmdir, rmdirat functions.
   *
   * \param path The name of the directory to remove.
   *             Note: \a path is relative to this
   *             directory and may contain subdirectories.
   * \return 0 on success, or <0 on error.
   */
  virtual int rmdir(const char *path) noexcept = 0;
  virtual int openat(const char *path, int flags, mode_t mode,
                     cxx::Ref_ptr<File> *f) noexcept = 0;

  virtual ssize_t getdents(char *buf, size_t sizebytes) noexcept = 0;

  virtual int fchmodat(const char *pathname,
                       mode_t mode, int flags) noexcept = 0;

  virtual int utimensat(const char *pathname,
                        const struct timespec times[2], int flags) noexcept = 0;

  /**
   * \internal
   */
  virtual int get_entry(const char *, int, mode_t, cxx::Ref_ptr<File> *) noexcept = 0;
};

inline
Directory::~Directory() noexcept
{}

/**
 * \brief Interface for a POSIX file that provides regular file semantics.
 *
 * Real objects always use the combined L4Re::Vfs::File interface.
 */
class Regular_file
{
public:
  virtual ~Regular_file() noexcept = 0;

  /**
   * \brief Get an L4Re::Dataspace object for the file.
   *
   * This is used as a backend for POSIX mmap and mmap2 functions.
   * \note mmap is not possible if the function returns an invalid
   *       capability.
   *
   * \return A capability to an L4Re::Dataspace that represents the file
   *         contents in an L4Re way.
   */
  virtual L4::Cap<L4Re::Dataspace> data_space() noexcept = 0;

  /**
   * \brief Read one or more blocks of data from the file.
   *
   * This function acts as backend for POSIX read and readv calls and
   * reads data starting from the f_pos pointer of that open file.
   * The file pointer is advanced according to the number of bytes read.
   *
   * \return The number of bytes read from the file, or <0 on error.
   */
  virtual ssize_t readv(const struct iovec*, int iovcnt) noexcept = 0;

  /**
   * \brief Write one or more blocks of data to the file.
   *
   * This function acts as backend for POSIX write and writev calls.
   * The data is written starting at the current file pointer and the
   * file pointer must be advanced according to the number of written
   * bytes.
   *
   * \return The number of bytes written to the file, or <0 on error.
   */
  virtual ssize_t writev(const struct iovec*, int iovcnt) noexcept = 0;

  virtual ssize_t preadv(const struct iovec *iov, int iovcnt, off64_t offset) noexcept = 0;
  virtual ssize_t pwritev(const struct iovec *iov, int iovcnt, off64_t offset) noexcept = 0;

  /**
   * \brief Change the file pointer.
   *
   * This is the backend for POSIX seek, lseek and friends.
   *
   * \return The new file position, or <0 on error.
   */
  virtual off64_t lseek64(off64_t, int) noexcept = 0;


  /**
   * \brief Truncate the file at the given position.
   *
   * This function is the backend for truncate and friends.
   * \param pos The offset at which the file shall be truncated.
   * \return 0 on success, or <0 on error.
   */
  virtual int ftruncate64(off64_t pos) noexcept = 0;

  /**
   * \brief Sync the data and meta data to persistent storage.
   *
   * This is the backend for POSIX fsync.
   */
  virtual int fsync() const noexcept = 0;

  /**
   * \brief Sync the data to persistent storage.
   *
   * This is the backend for POSIX fdatasync.
   */
  virtual int fdatasync() const noexcept = 0;

  /**
   * \brief Test if the given lock can be placed in the file.
   *
   * This function is used as backend for fcntl F_GETLK commands.
   * \param lock The lock that shall be placed on the file. The
   *             \a l_type member will contain `F_UNLCK` if the lock
   *             could be placed.
   * \return 0 on success, <0 on error.
   */
  virtual int get_lock(struct flock64 *lock) noexcept = 0;

  /**
   * \brief Acquire or release the given lock on the file.
   *
   * This function is used as backend for fcntl F_SETLK and F_SETLKW commands.
   * \param lock The lock that shall be placed on the file.
   * \param wait If true, then block if there is a conflicting lock on the file.
   * \return 0 on success, <0 on error.
   */
  virtual int set_lock(struct flock64 *lock, bool wait) noexcept = 0;
};

inline
Regular_file::~Regular_file() noexcept
{}

class Socket
{
public:
  virtual ~Socket() noexcept = 0;
  virtual int bind(sockaddr const *, socklen_t) noexcept = 0;
  virtual int connect(sockaddr const *, socklen_t) noexcept = 0;
  virtual ssize_t send(void const *, size_t, int) noexcept = 0;
  virtual ssize_t recv(void *, size_t, int) noexcept = 0;
  virtual ssize_t sendto(void const *, size_t, int, sockaddr const *, socklen_t) noexcept = 0;
  virtual ssize_t recvfrom(void *, size_t, int, sockaddr *, socklen_t *) noexcept = 0;
  virtual ssize_t sendmsg(msghdr const *, int) noexcept = 0;
  virtual ssize_t recvmsg(msghdr *, int) noexcept = 0;
  virtual int getsockopt(int level, int opt, void *, socklen_t *) noexcept = 0;
  virtual int setsockopt(int level, int opt, void const *, socklen_t) noexcept = 0;
  virtual int listen(int) noexcept = 0;
  virtual int accept(sockaddr *addr, socklen_t *) noexcept = 0;
  virtual int shutdown(int) noexcept = 0;

  virtual int getsockname(sockaddr *, socklen_t *) noexcept = 0;
  virtual int getpeername(sockaddr *, socklen_t *) noexcept = 0;
};

inline
Socket::~Socket() noexcept
{}

/**
 * \brief Interface for a POSIX file that provides special file semantics.
 *
 * Real objects always use the combined L4Re::Vfs::File interface.
 */
class Special_file
{
public:
  virtual ~Special_file() noexcept = 0;

  /**
   * \brief The famous IO control.
   *
   * Backend for POSIX generic object invocation ioctl.
   *
   * \param cmd The ioctl command.
   * \param args The arguments for the ioctl, usually some kind
   *             of pointer.
   * \return >=0 on success, or <0 on error.
   */
  virtual int ioctl(unsigned long cmd, va_list args) noexcept = 0;
};

inline
Special_file::~Special_file() noexcept
{}

/**
 * \brief The basic interface for an open POSIX file.
 *
 * An open POSIX file can be anything that hides behind a
 * POSIX file descriptor. This means that even directories
 * are files.  An open file can be anything from a directory to a
 * special device file so see Generic_file, Regular_file, Directory,
 * and Special_file for more information.
 *
 * \note For implementing a backend for the L4Re::Vfs
 *       L4Re::Vfs::Be_file may be used as a base class.
 *
 */
class File :
  public Generic_file,
  public Regular_file,
  public Directory,
  public Special_file,
  public Socket
{
  friend class Mount_tree;

private:
  void operator = (File const &);

protected:
  File() noexcept : _ref_cnt(0) {}
  File(File const &)
  : Generic_file(),Regular_file(), Directory(), Special_file(), _ref_cnt(0)
  {}

public:

  const char *get_mount(const char *path, cxx::Ref_ptr<File> *dir,
                        cxx::Ref_ptr<Mount_tree> *mt = 0) noexcept;

  int openat(const char *path, int flags, mode_t mode,
             cxx::Ref_ptr<File> *f) noexcept override;

  void add_ref() noexcept { ++_ref_cnt; }
  int remove_ref() noexcept { return --_ref_cnt; }

  virtual ~File() noexcept = 0;

  cxx::Ref_ptr<Mount_tree> mount_tree() const noexcept
  { return _mount_tree; }

private:
  int _ref_cnt;
  cxx::Ref_ptr<Mount_tree> _mount_tree;
};

inline
File::~File() noexcept
{}

class Path
{
private:
  char const *_p;
  unsigned _l;

public:
  Path() noexcept : _p(0), _l(0) {}

  explicit Path(char const *p) noexcept : _p(p)
  { for (_l = 0; *p; ++p, ++_l) ; }

  Path(char const *p, unsigned l) noexcept : _p(p), _l(l)
  {}

  static bool __is_sep(char s) noexcept;

  Path cmp_path(char const *prefix) const noexcept;

  struct Invalid_ptr;
  operator Invalid_ptr const * () const
  { return reinterpret_cast<Invalid_ptr const *>(_p); }

  unsigned length() const { return _l; }
  char const *path() const { return _p; }

  bool empty() const { return _l == 0; }

  bool is_sep(unsigned offset) const { return __is_sep(_p[offset]); }

  bool strip_sep()
  {
    bool s = false;
    for (; __is_sep(*_p) && _l; ++_p, --_l)
      s = true;
    return s;
  }

  Path first() const
  {
    unsigned i;
    for (i = 0; i < _l && !is_sep(i); ++i)
      ;

    return Path(_p, i);
  }

  Path strip_first()
  {
    Path r = first();
    _p += r.length();
    _l -= r.length();
    strip_sep();
    return r;
  }

};


/**
 * \internal
 * \brief Internal representation for a tree of mount points.
 * \note It should never be required to deal with Mount_tree objects
 *       directly.
 */
class Mount_tree
{
public:

  explicit Mount_tree(char *n) noexcept;

  Path lookup(Path const &path, cxx::Ref_ptr<Mount_tree> *mt,
              cxx::Ref_ptr<Mount_tree> *mp = 0) noexcept;

  Path find(Path const &p, cxx::Ref_ptr<Mount_tree> *t) noexcept;

  cxx::Ref_ptr<File> mount() const
  { return _mount; }

  void mount(cxx::Ref_ptr<File> const &m)
  {
    m->_mount_tree = cxx::ref_ptr(this);
    _mount = m;
  }

  static int create_tree(cxx::Ref_ptr<Mount_tree> const &root,
                         char const *path,
                         cxx::Ref_ptr<File> const &dir) noexcept;

  void add_child_node(cxx::Ref_ptr<Mount_tree> const &cld);

  virtual ~Mount_tree() noexcept  = 0;

  void add_ref() noexcept { ++_ref_cnt; }
  int remove_ref() noexcept { return --_ref_cnt; }

private:
  friend class Real_mount_tree;

  int _ref_cnt;
  char *_name;
  cxx::Ref_ptr<Mount_tree> _cld;
  cxx::Ref_ptr<Mount_tree> _sib;
  cxx::Ref_ptr<File> _mount;
};

inline
Mount_tree::~Mount_tree() noexcept
{}

inline bool
Path::__is_sep(char s) noexcept
{ return s == '/'; }

inline Path
Path::cmp_path(char const *n) const noexcept
{
  char const *p = _p;
  for (; *p && !__is_sep(*p) && *n; ++p, ++n)
    if (*p != *n)
      return Path();

  if (*n || (*p && !__is_sep(*p)))
    return Path();

  return Path(p, _l - (p - _p));
}

inline
Mount_tree::Mount_tree(char *n) noexcept
: _ref_cnt(0), _name(n)
{}

inline Path
Mount_tree::find(Path const &p, cxx::Ref_ptr<Mount_tree> *t) noexcept
{
  if (!_cld)
    return Path();

  for (cxx::Ref_ptr<Mount_tree> x = _cld; x; x = x->_sib)
    {
      Path const r = p.cmp_path(x->_name);
      if (r)
	{
	  *t = x;
	  return r;
	}
    }

  return Path();
}

inline Path
Mount_tree::lookup(Path const &path, cxx::Ref_ptr<Mount_tree> *mt,
                   cxx::Ref_ptr<Mount_tree> *mp) noexcept
{
  cxx::Ref_ptr<Mount_tree> x(this);
  Path p = path;

  if (p.first().cmp_path("."))
    p.strip_first();

  Path last_mp = p;

  if (mp)
    *mp = x;;

  while (1)
    {
      Path r = x->find(p, &x);

      if (!r)
	{
	  if (mp)
	    return last_mp;

	  if (mt)
	    *mt = x;

	  return p;
	}

      r.strip_sep();

      if (mp && x->_mount)
	{
	  last_mp = r;
	  *mp = x;
	}

      if (r.empty())
	{
	  if (mt)
	    *mt = x;

	  if (mp)
	    return last_mp;
	  else
	    return r;
	}

      p = r;
    }
}

inline
void
Mount_tree::add_child_node(cxx::Ref_ptr<Mount_tree> const &cld)
{
  cld->_sib = _cld;
  _cld = cld;
}

inline
const char *
File::get_mount(const char *path, cxx::Ref_ptr<File> *dir,
                cxx::Ref_ptr<Mount_tree> *mt) noexcept
{
  if (!_mount_tree)
    {
      *dir = cxx::ref_ptr(this);
      return path;
    }

  cxx::Ref_ptr<Mount_tree> mp;
  Path p = _mount_tree->lookup(Path(path), mt, &mp);
  if (mp->mount())
    {
      *dir = mp->mount();
      return p.path();
    }
  else
    {
      *dir = cxx::ref_ptr(this);
      return path;
    }
}

inline int
File::openat(const char *path, int flags, mode_t mode,
             cxx::Ref_ptr<File> *f) noexcept
{
  cxx::Ref_ptr<File> dir;
  cxx::Ref_ptr<Mount_tree> mt;
  path = get_mount(path, &dir, &mt);

  int res = dir->get_entry(path, flags, mode, f);

  if (res < 0)
    return res;

  if (!(*f)->_mount_tree && mt)
    (*f)->_mount_tree = mt;

  return res;
}

/**
 * \brief Interface for POSIX memory management.
 * \note This interface usually exists as a singleton and as a superclass
 *       of L4Re::Vfs::Ops.
 *
 * An implementation for this interface is in l4/l4re_vfs/impl/vfs_impl.h
 * and used by the l4re_vfs library or by the VFS implementation in ldso.
 */
class Mman
{
public:
  /// Backend for the mmap2 system call.
  virtual int mmap2(void *start, size_t len, int prot, int flags, int fd,
                    off_t offset, void **ptr) noexcept = 0;

  /// Backend for the munmap system call.
  virtual int munmap(void *start, size_t len) noexcept = 0;

  /// Backend for the mremap system call.
  virtual int mremap(void *old, size_t old_sz, size_t new_sz, int flags,
                     void **new_addr) noexcept = 0;

  /// Backend for the mprotect system call.
  virtual int mprotect(const void *a, size_t sz, int prot) noexcept = 0;

  /// Backend for the msync system call
  virtual int msync(void *addr, size_t len, int flags) noexcept = 0;

  /// Backend for the madvice system call
  virtual int madvise(void *addr, size_t len, int advice) noexcept = 0;

  virtual ~Mman() noexcept = 0;
};

inline
Mman::~Mman() noexcept {}

class File_factory
{
private:
  int _ref_cnt = 0;
  int _proto = 0;
  char const *_proto_name = 0;

  template<typename T> friend struct cxx::Default_ref_counter;
  void add_ref() noexcept { ++_ref_cnt; }
  int remove_ref() noexcept { return --_ref_cnt; }

public:
  explicit File_factory(int proto) : _proto(proto) {}
  explicit File_factory(char const *proto_name) : _proto_name(proto_name) {}
  File_factory(int proto, char const *proto_name)
  : _proto(proto), _proto_name(proto_name)
  {}

  File_factory(File_factory const &) = delete;
  File_factory &operator = (File_factory const &) = delete;

  char const *proto_name() const { return _proto_name; }
  int proto() const { return _proto; }

  virtual ~File_factory() noexcept = 0;
  virtual cxx::Ref_ptr<File> create(L4::Cap<void> file) = 0;
};

inline File_factory::~File_factory() noexcept {}

template<typename IFACE, typename IMPL>
class File_factory_t : public File_factory
{
public:
  File_factory_t()
  : File_factory(IFACE::Protocol, L4::kobject_typeid<IFACE>()->name())
  {}

  cxx::Ref_ptr<File> create(L4::Cap<void> file) override
  { return cxx::make_ref_obj<IMPL>(L4::cap_cast<IFACE>(file)); }
};

/**
 * \brief Basic interface for an L4Re::Vfs file system.
 * \note For implementing a special file system
 *       L4Re::Vfs::Be_file_system may be used as a base class.
 *
 * The main purpose of this interface is to have a
 * single object for each supported file-system type (e.g., ext2, vfat)
 * that exists in the application and is registered at the L4Re::Vfs::Fs
 * singleton available via L4Re::Vfs::vfs_ops.
 * Ultimately, the POSIX mount function calls the File_system::mount method
 * matching the file-system type given in mount.
 *
 */
class File_system
{
protected:
  File_system *_next;

public:
  File_system() noexcept : _next(0) {}
  /**
   * \brief Returns the type of the file system used in mount as fstype
   * argument.
   * \note This method is already provided by Be_file_system.
   */
  virtual char const *type() const noexcept = 0;

  /**
   * \brief Create a directory object \a dir representing \a source
   *        mounted with this file system.
   *
   * \param      source      The path to the source device to mount. This may
   *                         also be some URL or anything file-system specific.
   * \param      mountflags  The mount flags as specified in the POSIX
   *                         mount call.
   * \param      data        The data as specified in the POSIX mount call. The
   *                         contents are file-system specific.
   * \param[out] dir         A new directory object representing the file-system
   *                         root directory.
   *
   * \return 0 on success, and <0 on error (e.g. -EINVAL).
   *
   */
  virtual int mount(char const *source, unsigned long mountflags,
                    void const *data, cxx::Ref_ptr<File> *dir) noexcept = 0;

  virtual ~File_system() noexcept = 0;

  /**
   * \internal
   * \brief Get the next file system in the internal registry.
   */
  File_system *next() const noexcept { return _next; }
  File_system *&next() noexcept { return _next; }
  void next(File_system *n) noexcept { _next = n; }
};

inline
File_system::~File_system() noexcept
{}

class File_system_list
{
public:
  class Iterator
  {
  public:
    explicit constexpr Iterator(File_system *c = nullptr) : _c(c) {}

    Iterator &operator++()
    {
      if (_c)
        _c = _c->next();
      return *this;
    }

    bool operator==(Iterator const &other) const { return _c == other._c; }
    bool operator!=(Iterator const &other) const { return _c != other._c; }
    File_system *operator*() const { return _c; }
    File_system *operator->() const { return _c; }

  private:
    File_system *_c;
  };

  File_system_list(File_system *head) : _head(head) {}

  constexpr Iterator begin() const
  { return Iterator(_head); }

  constexpr Iterator end() const
  { return Iterator(); }

private:
  File_system *_head;
};

/**
 * \brief POSIX File-system related functionality.
 * \note This class usually exists as a singleton and as a superclass
 *       of L4Re::Vfs::Ops (\see L4Re::Vfs::vfs_ops).
 */
class Fs
{
public:
  /**
   * \brief Get the L4Re::Vfs::File for the file descriptor \a fd.
   * \param fd The POSIX file descriptor number.
   * \return A pointer to the File object, or 0 if \a fd is not open.
   */
  virtual cxx::Ref_ptr<File> get_file(int fd) noexcept = 0;

  /// Get the directory object for the application's root directory.
  virtual cxx::Ref_ptr<File> get_root() noexcept = 0;

  /// Get the directory object for the application's current working directory.
  virtual cxx::Ref_ptr<File> get_cwd() noexcept { return get_root(); }

  /// Set the current working directory for the application.
  virtual void set_cwd(cxx::Ref_ptr<File> const &) noexcept {}

  /**
   * \brief Allocate the next free file descriptor.
   * \param f The file to assign to that file descriptor.
   * \return The allocated file descriptor, or -EMFILE on error.
   */
  virtual int alloc_fd(cxx::Ref_ptr<File> const &f = cxx::Ref_ptr<>::Nil) noexcept = 0;

  /**
   * \brief Set the file object referenced by the file descriptor \a fd.
   * \param fd The file descriptor to set to \a f.
   * \param f The file object to assign.
   * \return A pair of a pointer to the file object that was previously
   *         assigned to fd (`first`) and a return value (`second`).
   *         `second` contains `-#EBADF` if the passed file descriptor is
   *         outside the valid range. `first` contains a Nil pointer in that
   *         case. On success, `second` contains 0.
   */
  virtual cxx::Pair<cxx::Ref_ptr<File>, int>
    set_fd(int fd, cxx::Ref_ptr<File> const &f = cxx::Ref_ptr<>::Nil) noexcept = 0;

  /**
   * \brief Free the file descriptor \a fd.
   * \param fd The file descriptor to free.
   * \return A pointer to the file object that was assigned to the fd.
   */
  virtual cxx::Ref_ptr<File> free_fd(int fd) noexcept = 0;

  /**
   * \brief Mount a given file object at the given global path in the VFS.
   * \param path The global path to mount \a dir at.
   * \param dir A pointer to the file/directory object that shall be mounted
   *            at \a path.
   * \return 0 on success, or <0 on error.
   */
  virtual int mount(char const *path, cxx::Ref_ptr<File> const &dir) noexcept = 0;

  /**
   * \internal
   * \brief Register a file-system type in the global registry.
   * \note This is done automatically by Be_file_system.
   * \param f A pointer to the file system to register.
   * \return 0 on success, or <0 on error.
   */
  virtual int register_file_system(File_system *f) noexcept = 0;

  /**
   * \internal
   * \brief Remove the given file system from the global registry.
   * \note This is done automatically by Be_file_system.
   * \param f The file system instance to remove from the registry.
   * \return 0 on success, <0 on error.
   */
  virtual int unregister_file_system(File_system *f) noexcept = 0;

  /**
   * \internal
   * \brief Find the file-system object for the given file-system type.
   * \note This function is used by the file-system mount call.
   * \param fstype The file-system type (e.g. ext, vfat).
   * \return A pointer to the file-system object, or 0 on error.
   */
  virtual File_system *get_file_system(char const *fstype) noexcept = 0;

  /**
   * \internal
   * \brief File-system iterator, get each file-system.
   * \note This function is used by the file-system mount call.
   * \param i Iterator, initialize with 'nullptr'. Done when returns
   *          'nullptr'.
   * \return A pointer to the file-system object, or 0 if done.
   */
  virtual File_system_list file_system_list() noexcept = 0;

  /**
   * \brief Backend for the POSIX mount call.
   */
  int mount(char const *source, char const *target,
            char const *fstype, unsigned long mountflags,
            void const *data) noexcept;

  virtual int register_file_factory(cxx::Ref_ptr<File_factory> f) noexcept = 0;
  virtual int unregister_file_factory(cxx::Ref_ptr<File_factory> f) noexcept = 0;
  virtual cxx::Ref_ptr<File_factory> get_file_factory(int proto) noexcept = 0;
  virtual cxx::Ref_ptr<File_factory> get_file_factory(char const *proto_name) noexcept = 0;

  virtual ~Fs() = 0;

private:
  int mount_one(char const *source, char const *target,
                File_system *fs, unsigned long mountflags,
                void const *data) noexcept;
};

inline int
Fs::mount_one(char const *source, char const *target,
              File_system *fs, unsigned long mountflags,
              void const *data) noexcept
{
  cxx::Ref_ptr<File> dir;
  int res = fs->mount(source, mountflags, data, &dir);

  if (res < 0)
    return res;

  return mount(target, dir);
}

inline int
Fs::mount(char const *source, char const *target,
          char const *fstype, unsigned long mountflags,
          void const *data) noexcept
{
  if (   fstype[0] == 'a'
      && fstype[1] == 'u'
      && fstype[2] == 't'
      && fstype[3] == 'o'
      && fstype[4] == 0)
    {
      File_system_list fsl = file_system_list();
      for (File_system_list::Iterator c = fsl.begin(); c != fsl.end(); ++c)
        if (mount_one(source, target, *c, mountflags, data) == 0)
          return 0;

      return -ENODEV;
    }

  File_system *fs = get_file_system(fstype);

  if (!fs)
    return -ENODEV;

  return mount_one(source, target, fs, mountflags, data);
}

inline
Fs::~Fs()
{}

/**
 * \brief Interface for the POSIX backends of an application.
 * \note There usually exists a single instance of this interface
 *       available via L4Re::Vfs::vfs_ops that is used for all
 *       kinds of C-Library functions.
 */
class Ops : public Mman, public Fs
{
public:
  virtual void *malloc(size_t bytes) noexcept = 0;
  virtual void free(void *mem) noexcept = 0;
  virtual ~Ops() noexcept = 0;

  char *strndup(char const *str, unsigned l) noexcept
  {
    unsigned len;
    for (len = 0; str[len] && len < l; ++len)
      ;

    if (len == 0)
      return nullptr;

    ++len;

    char *b = static_cast<char *>(this->malloc(len));
    if (b == nullptr)
      return nullptr;

    char *r = b;
    for (; len - 1 > 0 && *str; --len, ++b, ++str)
      *b = *str;

    *b = 0;
    return r;
  }

};

inline
Ops::~Ops() noexcept
{}

}}

#endif

