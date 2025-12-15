#include "libc-api.h"

#include <tls.h>
#include <stdio.h>
#include <string.h>

#include <l4/sys/compiler.h>
#include <l4/sys/kdebug.h>

#include <ldsodefs.h>
#include <stdbool.h>

extern pthread_descr __pthread_main_thread;

#if defined SHARED
/* When using __thread for this, we do it in libc so as not
   to give libpthread its own TLS segment just for this.  */
extern void **__libc_dl_error_tsd (void) __attribute__ ((const));
#endif

static __inline__ void __attribute__((always_inline))
init_one_static_tls (pthread_descr descr, struct link_map *map)
{
#if defined(TLS_TCB_AT_TP)
  dtv_t *dtv = GET_DTV (descr);
  void *dest = (char *) descr - map->l_tls_offset;
#elif defined(TLS_DTV_AT_TP)
  dtv_t *dtv = GET_DTV ((pthread_descr) ((char *) descr + TLS_PRE_TCB_SIZE));
  void *dest = (char *) descr + map->l_tls_offset + TLS_PRE_TCB_SIZE;
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

static void
__pthread_init_static_tls (struct link_map *map)
{
  pthread_descr th;

  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive)
    {
      init_one_static_tls(th, map);
    }
}

int
ptlc_become_threaded(void)
{
  return 0; // Hook not needed for uclibc-ng
}

void *
ptlc_allocate_tls(void)
{
  return _dl_allocate_tls(NULL);
}

void
ptlc_deallocate_tls(void *tcb)
{
  _dl_deallocate_tls(tcb, true);
}

extern void __libc_setup_tls (size_t tcbsize, size_t tcbalign);

void
ptlc_init_static_tls(__attribute__((unused)) void *arg)
{
#ifndef SHARED
  __libc_setup_tls(TLS_TCB_SIZE, TLS_TCB_ALIGN);
#endif
}

pthread_descr
ptlc_thread_descr_self(void)
{
  return THREAD_SELF;
}

pthread_descr
ptlc_tls_tp_to_thread_descr(void *tls_tp) //DONE!
{
#ifdef TLS_DTV_AT_TP
  return (pthread_descr)((char *)tls_tp - TLS_PRE_TCB_SIZE);
#else
  return (pthread_descr)tls_tp;
#endif
}

void *
ptlc_thread_descr_to_tls_tp(pthread_descr descr)
{
#ifdef TLS_DTV_AT_TP
  return (void *)((char *)descr + TLS_PRE_TCB_SIZE);
#else
  return (void *)descr;
#endif
}

void
ptlc_before_create_thread(void)
{} // Not needed for uclibc-ng

void
ptlc_after_create_thread(__attribute__((unused)) bool success)
{} // Not needed for uclibc-ng

void
ptlc_after_exit_thread(void)
{} // Not needed for uclibc-ng

int
ptlc_set_tp(void *tls_tp)
{
  TLS_INIT_TP(tls_tp, 0);
  return 0;
}

void
ptlc_after_pthread_initialize(void)
{
#if defined SHARED
  /* Transfer the old value from the dynamic linker's internal location.  */
  *__libc_dl_error_tsd () = *(*GL(dl_error_catch_tsd)) ();
  GL(dl_error_catch_tsd) = &__libc_dl_error_tsd;

  /* Make __rtld_lock_{,un}lock_recursive use pthread_mutex_{,un}lock,
     keep the lock count from the ld.so implementation.  */
  GL(dl_rtld_lock_recursive) = (void *) __pthread_mutex_lock;
  GL(dl_rtld_unlock_recursive) = (void *) __pthread_mutex_unlock;
  unsigned int rtld_lock_count = GL(dl_load_lock).__m_count;
  GL(dl_load_lock).__m_count = 0;
  while (rtld_lock_count-- > 0)
    __pthread_mutex_lock (&GL(dl_load_lock));
#endif

  GL(dl_init_static_tls) = &__pthread_init_static_tls;

  /* uClibc-specific stdio initialization for threads. */
  {
    FILE *fp;
    _stdio_user_locking = 0;       /* 2 if threading not initialized */
    for (fp = _stdio_openlist; fp != NULL; fp = fp->__nextopen) {
      if (fp->__user_locking != 1) {
        fp->__user_locking = 0;
      }
    }
  }
}
