/*
 * Support for dynamic linking code in static libc.
 * Copyright (C) 1996-2002, 2003, 2004, 2005 Free Software Foundation, Inc.
 *
 * Partially based on GNU C Library (file: libc/elf/dl-support.c)
 *
 * Copyright (C) 2008 STMicroelectronics Ltd.
 * Author: Carmelo Amoroso <carmelo.amoroso@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#include <link.h>
#include <ldso.h>
#include <elf.h>
#if defined(__UCLIBC_HAS_THREADS__)
#include <bits/libc-lock.h>
#endif
#if defined(USE_TLS) && USE_TLS
#include <assert.h>
#include <tls.h>
#include <stdbool.h>
#include <ldsodefs.h>
#include <string.h>
#endif
#include <bits/uClibc_page.h>

#if defined(USE_TLS) && USE_TLS

void (*_dl_init_static_tls) (struct link_map *) = &_dl_nothread_init_static_tls;

#endif

ElfW(Phdr) *_dl_phdr;
size_t _dl_phnum;
size_t _dl_pagesize;

ElfW(auxv_t) _dl_auxvt[AUX_MAX_AT_ID];
ElfW(auxv_t) *_dl_auxv_start;

#ifndef __NOT_FOR_L4__
extern void *l4re_global_env __attribute__((weak));
extern void *l4_global_kip __attribute__((weak));
#endif

void internal_function _dl_aux_init (ElfW(auxv_t) *av);
void internal_function _dl_aux_init (ElfW(auxv_t) *av)
{
   _dl_auxv_start = av;

   memset(_dl_auxvt, 0x00, sizeof(_dl_auxvt));
   for (; av->a_type != AT_NULL; av++)
     {
       if (av->a_type < AUX_MAX_AT_ID)
         _dl_auxvt[av->a_type] = *av;

#ifndef __NOT_FOR_L4__
       if (av->a_type == 0xf1 && &l4re_global_env)
         l4re_global_env = (void *)av->a_un.a_val;
       else if (av->a_type == 0xf2 && &l4_global_kip)
         l4_global_kip = (void *)av->a_un.a_val;
#endif
     }

   /* Get the program headers base address from the aux vect */
   _dl_phdr = (ElfW(Phdr) *) _dl_auxvt[AT_PHDR].a_un.a_val;

   /* Get the number of program headers from the aux vect */
   _dl_phnum = (size_t) _dl_auxvt[AT_PHNUM].a_un.a_val;

   /* Get the pagesize from the aux vect */
   _dl_pagesize = (_dl_auxvt[AT_PAGESZ].a_un.a_val) ? (size_t) _dl_auxvt[AT_PAGESZ].a_un.a_val : PAGE_SIZE;
}

#if defined(USE_TLS) && USE_TLS
/* Initialize static TLS area and DTV for current (only) thread.
   libpthread implementations should provide their own hook
   to handle all threads.  */
void
attribute_hidden
_dl_nothread_init_static_tls (struct link_map *map)
{
# if defined(TLS_TCB_AT_TP)
  void *dest = (char *) THREAD_SELF - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  void *dest = (char *) THREAD_SELF + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv_t *dtv = THREAD_DTV ();
  assert (map->l_tls_modid <= dtv[-1].counter);
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

#endif

#if defined(__UCLIBC_HAS_THREADS__)
__rtld_lock_define_initialized_recursive (, _dl_load_lock)
__rtld_lock_define_initialized_recursive (, _dl_load_write_lock)
#endif
