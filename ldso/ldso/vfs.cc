// we do better not export anything from LDSO

#define L4_EXPORT

#include <l4/crtn/initpriorities.h>
#include <l4/util/util.h>

#include <l4/re/env>
#include <l4/re/util/bitmap_cap_alloc>

#include <stddef.h>
#include <dlfcn.h>

inline
void l4_sleep(int)
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
namespace Vfs_config {

  using ::memcpy;
  typedef void *Dl_open(const char *libname, int flag);
  static Dl_open *_dl_open;

  namespace
  {
    L4Re::Util::Cap_alloc<256> __attribute__((init_priority(INIT_PRIO_L4RE_UTIL_CAP_ALLOC))) __cap_alloc(256); //L4Re::Env::env()->first_free_cap() - 256);
  };
  L4Re::Util::Cap_alloc_base &cap_alloc = __cap_alloc;


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
        _dl_open = (Dl_open*)_dl_find_hash("dlopen", _dl_symbol_tables, NULL, ELF_RTYPE_CLASS_PLT, NULL);
	if (!_dl_open)
	  return -1;
      }

    char const prefix[] = "libl4revfs-fs-";
    char const suffix[] = ".so";
    char name[80];
    memcpy(name, prefix, sizeof(prefix) - 1);
    memcpy(name + sizeof(prefix) - 1, fstype, strlen(fstype));
    memcpy(name + sizeof(prefix) + strlen(fstype) - 1, suffix, sizeof(suffix));
    return _dl_open(name, RTLD_LOCAL | RTLD_LAZY) ? 0 : -1;
  }

  inline void *malloc(size_t size) { return _dl_malloc(size); }
  inline void free(void *p) { _dl_free(p); }

}


extern "C" void __cxa_pure_virtual(void)
{
  l4_sleep_forever();
}

extern "C" void __cxa_atexit(void)
{}

#ifdef __ARM_EABI__
extern "C" void __aeabi_atexit(void)
{}
#endif

#include <l4/l4re_vfs/impl/ns_fs_impl.h>
#include <l4/l4re_vfs/impl/ro_file_impl.h>
#include <l4/l4re_vfs/impl/fd_store_impl.h>
#include <l4/l4re_vfs/impl/vcon_stream_impl.h>
#include <l4/l4re_vfs/impl/vfs_api_impl.h>
#include <l4/l4re_vfs/impl/vfs_impl.h>
// must be the last
#include <l4/l4re_vfs/impl/default_ops_impl.h>
