// we do better not export anything from LDSO
#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#define L4_EXPORT

#include <l4/crtn/initpriorities.h>
#include <l4/util/util.h>

#include <l4/cxx/static_container>
#include <l4/re/env>
#include <l4/re/util/bitmap_cap_alloc>
#include <l4/re/cap_alloc>
#include <stdlib.h>

#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <stddef.h>
#include <dlfcn.h>

inline
void l4_sleep(l4_uint32_t) L4_NOTHROW
{}

// must correspond to dl-elf.h
#define ELF_RTYPE_CLASS_PLT (0x1)

// must correspond to dl-hash.h
struct dyn_elf {
  struct elf_resolve * dyn;
};

extern struct dyn_elf *_dl_symbol_tables;
extern "C" char * _dl_find_hash(const char * name, struct dyn_elf * rpnt,
				  struct elf_resolve *mytpnt, int type_class,
				  struct elf_resolve **tpntp);

extern "C" void _dl_dprintf(int, const char *, ...);
extern "C" void *_dl_malloc(size_t size);
extern "C" void _dl_free(void *m);

namespace L4Re {
  Cap_alloc *virt_cap_alloc asm ("l4re_vfs_virt_cap_alloc");

  extern Cap_alloc *__rtld_l4re_virt_cap_alloc
    __attribute__((alias("l4re_vfs_virt_cap_alloc"), visibility("default")));
}

namespace Vfs_config {

  using ::memcpy;
  typedef void *Dl_open(const char *libname, int flag);
  static Dl_open *_dl_open;

  namespace
  {
    enum : unsigned { Num_caps = 256 };
    typedef L4Re::Cap_alloc_t<L4Re::Util::Cap_alloc<Num_caps> > Cap_alloc;

    // small ldso-internal cap allocator for the first libs
    cxx::Static_container<Cap_alloc> __cap_alloc;

    static void init()
    {
      auto *env = reinterpret_cast<L4Re::Env *>(l4re_global_env);
      __cap_alloc.construct(env->first_free_cap());
      env->first_free_cap(env->first_free_cap() + Num_caps);

      // Use compile-time version first. Later replaced by
      // L4Re::Util::cap_alloc during libc initialization.
      ::L4Re::virt_cap_alloc = __cap_alloc.get();
    }

    L4_DECLARE_CONSTRUCTOR(init, INIT_PRIO_L4RE_UTIL_CAP_ALLOC);
  }

  inline
  L4::Cap<L4Re::Mem_alloc> allocator()
  {
    return reinterpret_cast<L4Re::Env const *>(l4re_global_env)->mem_alloc();
  }

  inline int
  load_module(char const *fstype)
  {
    // We do not support loading extra file system modules for now in musl
    return -ENOTSUP;
  }

  // Since ldso is part of libc we can just use malloc & free
  using ::malloc;
  using ::free;
}

extern "C" void __cxa_pure_virtual(void);
extern "C" void __cxa_pure_virtual(void)
{
  l4_sleep_forever();
}

#ifdef __ARM_EABI__
extern "C" void __aeabi_atexit(void);
extern "C" void __aeabi_atexit(void)
{}
#endif

#include <l4/l4re_vfs/impl/ns_fs_impl.h>
#include <l4/l4re_vfs/impl/ro_file_impl.h>
#include <l4/l4re_vfs/impl/fd_store_impl.h>
#include <l4/l4re_vfs/impl/vcon_stream_impl.h>
#include <l4/l4re_vfs/impl/vfs_impl.h>
// must be the last
#include <l4/l4re_vfs/impl/default_ops_impl.h>
