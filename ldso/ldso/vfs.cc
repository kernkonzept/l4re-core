// we do better not export anything from LDSO

#define L4_EXPORT

// Don't create references to L4Re::Util::Dbg implementations. Logging is not
// used here, therefore libld-l4.so is not linked against lib4re-util.
//
// Unfortunately gcc-16 is unable to remove unused code dragged in by
// <l4/re/util/env_ns> dragged in by <l4/l4re_vfs/impl/ns_fs_impl.h> unless
// -fno-devirtualize-speculatively is passed to the compiler. See
//
//   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124182.
//
// The actual trigger is
//
//   L4Re::Util::Env:ns::Env_ns(Env const *env = Env::env(),
//                              L4Re::Cap_alloc *ca = &L4Re::Util::cap_alloc)
//
// which drags in L4Re::Util::cap_alloc (Counting_cap_alloc) although this
// allocator is not needed.
#undef NDEBUG
#define NDEBUG

#include <l4/crtn/initpriorities.h>
#include <l4/util/util.h>

#include <l4/cxx/static_container>
#include <l4/re/env>
#include <l4/re/util/bitmap_cap_alloc>
#include <l4/re/cap_alloc>

#include <stddef.h>
#include <dlfcn.h>

inline
void l4_sleep(l4_uint32_t) L4_NOTHROW
{}


#include <l4/re/impl/dataspace_impl.h>
#include <l4/re/impl/rm_impl.h>
#include <l4/re/impl/namespace_impl.h>
#include <l4/re/impl/mem_alloc_impl.h>

// must correspond to dl-elf.h
#define ELF_RTYPE_CLASS_PLT (0x1)

// must correspond to dl-hash.h
struct dyn_elf {
  struct elf_resolve * dyn;
#if 0
  struct dyn_elf * next_handle;  /* Used by dlopen et al. */
  struct init_fini init_fini;
  struct dyn_elf * next;
  struct dyn_elf * prev;
#endif
};

extern struct dyn_elf *_dl_symbol_tables;
extern "C" char * _dl_find_hash(const char * name, struct dyn_elf * rpnt,
				  struct elf_resolve *mytpnt, int type_class,
				  struct elf_resolve **tpntp);

extern "C" attribute_hidden void *__rtld_l4re_global_env;
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
      auto *env = reinterpret_cast<L4Re::Env *>(__rtld_l4re_global_env);
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
    return reinterpret_cast<L4Re::Env const *>(__rtld_l4re_global_env)->mem_alloc();
  }

  inline int
  load_module(char const *fstype)
  {
    if (!_dl_open)
      {
        char *open = _dl_find_hash("dlopen", _dl_symbol_tables, NULL,
                                   ELF_RTYPE_CLASS_PLT, NULL);
        _dl_open = reinterpret_cast<Dl_open*>(open);
        if (!_dl_open)
          return -1;
      }

    if (fstype[0] == '\0')
      return -1;

    char const prefix[] = "libl4revfs-fs-";
    char const suffix[] = ".so";
    char name[80];
    size_t fstype_len = strlen(fstype);

    if (fstype_len > sizeof(name) - (sizeof(prefix) - 1)
                                  - (sizeof(suffix) - 1) - 1)
      return -1;

    memcpy(name, prefix, sizeof(prefix) - 1);
    memcpy(name + sizeof(prefix) - 1, fstype, fstype_len);
    memcpy(name + sizeof(prefix) - 1 + fstype_len, suffix, sizeof(suffix));
    return _dl_open(name, RTLD_LOCAL | RTLD_LAZY) ? 0 : -1;
  }

  inline void *malloc(size_t size) { return _dl_malloc(size); }
  inline void free(void *p) { _dl_free(p); }

}

extern "C" void __cxa_pure_virtual(void);
extern "C" void __cxa_pure_virtual(void)
{
  l4_sleep_forever();
}

extern "C" void __cxa_atexit(void);
extern "C" void __cxa_atexit(void)
{}

#ifdef __ARM_EABI__
extern "C" void __aeabi_atexit(void);
extern "C" void __aeabi_atexit(void)
{}
#endif

// Another workaround for gcc-16 being unable to remove dead code under certain
// conditions, see explanations on top of this file.
void operator delete (void *) noexcept
{ __builtin_trap(); }
void operator delete (void *, size_t) noexcept
{ __builtin_trap(); }

#include <l4/l4re_vfs/impl/ns_fs_impl.h>
#include <l4/l4re_vfs/impl/ro_file_impl.h>
#include <l4/l4re_vfs/impl/fd_store_impl.h>
#include <l4/l4re_vfs/impl/vcon_stream_impl.h>
#include <l4/l4re_vfs/impl/vfs_impl.h>
// must be the last
#include <l4/l4re_vfs/impl/default_ops_impl.h>
