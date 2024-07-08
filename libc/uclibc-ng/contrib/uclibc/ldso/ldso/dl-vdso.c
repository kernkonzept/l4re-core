
#include <elf.h>
#include <string.h>
#include "sys/auxv.h"

#define __ARCH_VDSO_GETTIMEOFDAY_NAME    "__vdso_gettimeofday"
#define __ARCH_VDSO_CLOCK_GETTIME_NAME   "__vdso_clock_gettime"

#if defined(__UCLIBC_USE_TIME64__)
#define __ARCH_VDSO_CLOCK_GETTIME64_NAME "__vdso_clock_gettime64"
#endif

/* Maybe override default vDSO functions names by arch-specific */
#include "ldso.h"
#include "generated/autoconf.h"

#ifndef ELF_BITS
# if ULONG_MAX > 0xffffffffUL
#  define ELF_BITS 64
# else
#  define ELF_BITS 32
# endif
#endif

#define ELF_BITS_XFORM2(bits, x) Elf##bits##_##x
#define ELF_BITS_XFORM(bits, x) ELF_BITS_XFORM2(bits, x)
#define ELF(x) ELF_BITS_XFORM(ELF_BITS, x)


#ifndef __VDSO_SUPPORT__
void load_vdso( void *sys_info_ehdr attribute_unused,
                char **envp attribute_unused ){
#ifdef __SUPPORT_LD_DEBUG__
    if ( _dl_debug_vdso != 0 ){
        _dl_dprintf(2,"_dl_vdso support not enabled\n" );
    }
#endif
}
#else

void *_dl__vdso_gettimeofday  = 0;
void *_dl__vdso_clock_gettime = 0;

#if defined(__UCLIBC_USE_TIME64__)
void *_dl__vdso_clock_gettime64 = 0;
#endif

void *_get__dl__vdso_clock_gettime(void);
void *_get__dl__vdso_clock_gettime(void)
{
    return _dl__vdso_clock_gettime;
}

#if defined(__UCLIBC_USE_TIME64__)
void *_get__dl__vdso_clock_gettime64(void);
void *_get__dl__vdso_clock_gettime64(void)
{
    return _dl__vdso_clock_gettime64;
}
#endif

void *_get__dl__vdso_gettimeofday(void);
void *_get__dl__vdso_gettimeofday(void)
{
    return _dl__vdso_gettimeofday;
}

typedef struct {

    void* base_addr;

    ELF(Ehdr) *hdr;

    char* section_header_strtab;

    ELF(Sym) *dynsym_table;
    uint32_t   dynsym_table_num;

    char* dynstr_table;

    uint16_t  *versym_table;

    ELF(Verdef) *verdef_table;
    uint32_t      verdef_num;

    ELF(Dyn) *dynamic_section;
    uint32_t  dynamic_section_num;

    char* text_section;


    char* vers_strings[10];


} elf_infos;

/*
 * the raise() dummy function is needed because of divisons in this code
 * but keep it hidden in this object
 *
 * fixes link error with gcc 12 for arm
 */
#pragma GCC visibility push(hidden)
int raise(int sig){
    sig = sig;
    return 0;
}
#pragma GCC visibility pop


static int vdso_check_elf_header( elf_infos* elf ){
    
    if ( 0 != _dl_memcmp( ELFMAG, elf->base_addr, 4 ) ){
        return 1;
    }
    
    if (elf->hdr->e_ident[EI_CLASS] != (ELF_BITS == 32 ? ELFCLASS32 : ELFCLASS64)) {
        _dl_dprintf(2,"vdso ELF Bits check error\n");
        return 1;  /* Wrong ELF class -- check ELF_BITS */
    }
    
    return 0;
}

static ELF(Shdr) *vdso_get_sec_header( elf_infos* elf, int index ){
    
    return (ELF(Shdr) *) ( elf->base_addr + elf->hdr->e_shoff + ( index * sizeof( ELF(Shdr) )) );
    
}

void load_vdso(void *sys_info_ehdr, char **envp ){

    elf_infos vdso_infos;

    if ( sys_info_ehdr == 0 ){
#ifdef __SUPPORT_LD_DEBUG__
        if ( _dl_debug_vdso != 0 ){
            _dl_dprintf(2,"_dl_vdso no vdso provied by kernel\n" );
        }
#endif
        return;
        
    }
    
    char* _dl_vdso_disable = _dl_getenv("VDSO_DISABLE", envp);
    if ( _dl_vdso_disable != 0 ){
#ifdef __SUPPORT_LD_DEBUG__
        if ( _dl_debug_vdso != 0 ){
            _dl_dprintf(2,"_dl_vdso vdso support disabled\n" );
        }
#endif
        return;
    }

    _dl_memset( &vdso_infos, 0 , sizeof( elf_infos ) );

    vdso_infos.base_addr = (void*)sys_info_ehdr;
    vdso_infos.hdr = (ELF(Ehdr)*)vdso_infos.base_addr;

    if ( 0 != vdso_check_elf_header( &vdso_infos ) ){
        return;
    }

    ELF(Shdr) *sec_header = vdso_get_sec_header( &vdso_infos, vdso_infos.hdr->e_shstrndx);
    vdso_infos.section_header_strtab = ( vdso_infos.base_addr + sec_header->sh_offset );
    
    /*
     * 
     * load ELF section headers
     * 
     */
    
    for ( int i = 0 ; i < vdso_infos.hdr->e_shnum; i++ ){
    
        sec_header = vdso_get_sec_header( &vdso_infos, i );
        
        char* name = vdso_infos.section_header_strtab + sec_header->sh_name;
        
        if( ( SHT_DYNSYM == sec_header->sh_type ) && ( 0 == _dl_strcmp( ".dynsym",name ) ) ){
            vdso_infos.dynsym_table = ( vdso_infos.base_addr + sec_header->sh_offset );
            vdso_infos.dynsym_table_num = sec_header->sh_size / sec_header->sh_entsize ;
            continue;
        }
        
        if( ( SHT_STRTAB == sec_header->sh_type ) && ( 0 == _dl_strcmp( ".dynstr",name ) ) ){
            vdso_infos.dynstr_table = ( vdso_infos.base_addr + sec_header->sh_offset );
            continue;
        }
        
        if( ( SHT_GNU_versym == sec_header->sh_type ) && ( 0 == _dl_strcmp( ".gnu.version",name ) ) ){
            vdso_infos.versym_table = ( vdso_infos.base_addr + sec_header->sh_offset );
            continue;
        }
        
        if( ( SHT_GNU_verdef == sec_header->sh_type ) && ( 0 == _dl_strcmp( ".gnu.version_d",name ) ) ){
            vdso_infos.verdef_table = ( vdso_infos.base_addr + sec_header->sh_offset );
            continue;
        }
        
        if( ( SHT_DYNAMIC == sec_header->sh_type ) && ( 0 == _dl_strcmp( ".dynamic",name ) ) ){
            vdso_infos.dynamic_section = ( vdso_infos.base_addr + sec_header->sh_offset );
            vdso_infos.dynamic_section_num = sec_header->sh_size / sec_header->sh_entsize ;
            continue;
        }
        
        if( ( SHT_PROGBITS == sec_header->sh_type ) && ( 0 == _dl_strcmp( ".text",name ) ) ){
            vdso_infos.text_section = ( vdso_infos.base_addr + sec_header->sh_offset );
            continue;
        }
       
    }


    /*
     * 
     * check section header -> dynamic table consistence
     * 
     */
    
    
    for( int i = 0 ; i < vdso_infos.dynamic_section_num ; i++ ){
        ELF(Dyn) *dyn_sec = &vdso_infos.dynamic_section[i];
        if ( dyn_sec->d_tag == 0 ) continue;
        
        
        if ( dyn_sec->d_tag == DT_STRTAB ){
            char* strtab = ( vdso_infos.base_addr + dyn_sec->d_un.d_ptr );
            if ( strtab != (char*) vdso_infos.dynstr_table ){
                _dl_dprintf(2,"vdso elf DT_STRTAB check error\n");
                return;
            }
            continue;
        }
        
        if ( dyn_sec->d_tag == DT_SYMTAB ){
            char* symtab = ( vdso_infos.base_addr + dyn_sec->d_un.d_ptr );
            if ( symtab != (char*) vdso_infos.dynsym_table ){
                _dl_dprintf(2,"vdso elf DT_SYMTAB check error\n");
                return;
            }
            continue;
        }
        
        if ( dyn_sec->d_tag == DT_VERDEF ){
            Elf32_Verdef* verdef = ( vdso_infos.base_addr + dyn_sec->d_un.d_ptr );
            if ( verdef != (Elf32_Verdef*) vdso_infos.verdef_table ){
                _dl_dprintf(2,"vdso elf DT_VERDEF check error\n");
                return;
            }
            continue;
        }
        
        if ( dyn_sec->d_tag == DT_VERDEFNUM ){
            vdso_infos.verdef_num = dyn_sec->d_un.d_val;
            continue;
        }
        
        if ( dyn_sec->d_tag == DT_VERSYM ){
            uint16_t*  versym = ( vdso_infos.base_addr + dyn_sec->d_un.d_ptr );
            if ( versym !=  vdso_infos.versym_table ){
                _dl_dprintf(2,"vdso elf DT_VERSYM check error\n");
                return;
            }
            continue;
        }
        
    }
    
    /*
     * 
     * load vdso version definition strings
     * 
     */
    
    ELF(Verdef) *vd = vdso_infos.verdef_table;
    for( int i = 0 ; i < vdso_infos.verdef_num ; i++ ){
        
        ELF(Verdaux) *vd_aux = (ELF(Verdaux)     *)(( ( char*)vd ) + vd->vd_aux);
        
        vdso_infos.vers_strings[ vd->vd_ndx ]   = vdso_infos.dynstr_table + vd_aux->vda_name;
        
        vd = ( ELF(Verdef) *)(( ( char*)vd ) + vd->vd_next);
        
    }
    
    /*
     * 
     * load function from the vdso
     * 
     */
#ifdef __SUPPORT_LD_DEBUG__ 
    if ( _dl_debug_vdso != 0 ){
        int vdso_functions = 0;
        for( int i = 0 ; i < vdso_infos.dynsym_table_num ; i++ ){
            ELF(Sym)* sym = &vdso_infos.dynsym_table[i];
            
            if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
                continue;
            
            char* name = vdso_infos.dynstr_table + sym->st_name;
            if ( name[0] == 0 ){
                continue;
            }

            vdso_functions++;
        }
        _dl_dprintf(2,"_dl_vdso_load functions found : %d\n", vdso_functions );
    }

#endif    
    
    for( int i = 0 ; i < vdso_infos.dynsym_table_num ; i++ ){
        ELF(Sym)* sym = &vdso_infos.dynsym_table[i];
        
        if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
            continue;

        char* name = vdso_infos.dynstr_table + sym->st_name;
        void* func_addr = (void*)( vdso_infos.base_addr + sym->st_value );
        
        // the function name is patched to zero if the kernel has no timer which is
        // usable for the function
        if ( name[0] == 0 ){
#ifdef __SUPPORT_LD_DEBUG__
            if ( _dl_debug_vdso != 0 ){
                _dl_dprintf(2,"   function at address %p disabled by the kernel\n", sym->st_value );
            }
#endif
            continue;
        }

        if ( 0 == _dl_strcmp( name, __ARCH_VDSO_GETTIMEOFDAY_NAME ) ){
            _dl__vdso_gettimeofday = func_addr;
#ifdef __SUPPORT_LD_DEBUG__
            if ( _dl_debug_vdso != 0 ){
                _dl_dprintf(2,"   %s at address %p\n", name, func_addr );
            }
#endif
            continue;
        }
        if ( 0 == _dl_strcmp( name, __ARCH_VDSO_CLOCK_GETTIME_NAME ) ){
            _dl__vdso_clock_gettime = func_addr;
#ifdef __SUPPORT_LD_DEBUG__
            if ( _dl_debug_vdso != 0 ){
                _dl_dprintf(2,"   %s at address %p\n", name, func_addr );
            }
#endif
            continue;
        }

#if defined(__UCLIBC_USE_TIME64__)
        if ( 0 == _dl_strcmp( name, __ARCH_VDSO_CLOCK_GETTIME64_NAME ) ){
            _dl__vdso_clock_gettime64 = func_addr;
#ifdef __SUPPORT_LD_DEBUG__
            if ( _dl_debug_vdso != 0 ){
                _dl_dprintf(2,"   %s at address %p\n", name, func_addr );
            }
#endif
            continue;
        }
#endif  /* defined(__UCLIBC_USE_TIME64__) */
        
#ifdef __SUPPORT_LD_DEBUG__
        if ( _dl_debug_vdso != 0 ){
            _dl_dprintf(2,"   <%s> not handled\n", name );
        }
#endif
    
    }
}

#endif // __VDSO_SUPPORT__
