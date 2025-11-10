#include "libc-api.h"
#include "pthread_impl.h"
#include "stdio_impl.h"

#include <l4/sys/compiler.h>
#include <l4/sys/kdebug.h>

#include <stdbool.h>

#define _UNIMPLEMENTED                                   \
  {                                                      \
    outstring("ptlc: Unimplemented ");                   \
    outstring(__func__);                                 \
    outstring("\n  Backtrace:");                           \
    outstring("\n  #1: ");                               \
    outumword((l4_umword_t)__builtin_return_address(0)); \
    outstring("\n");                                     \
  }

static FILE *volatile dummy_file = 0;
weak_alias(dummy_file, __stdin_used);
weak_alias(dummy_file, __stdout_used);
weak_alias(dummy_file, __stderr_used);

static void init_file_lock(FILE *f)
{
  if (f && f->needs_lock < 0)
    {
      f->needs_lock = 0;
      f->lock = FILE_LOCK_INITIALIZER;
    }
}

int
ptlc_become_threaded()
{
  // NOTE: Taken from __pthread_create()

  if (!libc.can_do_threads)
    return ENOSYS;

  if (libc.threaded)
    // Already threaded.
    return 0;

  for (FILE *f = *__ofl_lock(); f; f = f->next)
    init_file_lock(f);
  __ofl_unlock();
  init_file_lock(__stdin_used);
  init_file_lock(__stdout_used);
  init_file_lock(__stderr_used);
  // TODO: Do we need to replicate the following operations?
  //__syscall(SYS_rt_sigprocmask, SIG_UNBLOCK, SIGPT_SET, 0, _NSIG / 8);
  // self->tsd = (void **)__pthread_tsd_main;
  //__membarrier_init();
  libc.threaded = 1;
  // Unlike musl, we never reset this to 0, even if all but one thread exit again.
  libc.need_locks = 1;
  return 0;
}

void *
ptlc_allocate_tls()
{
  // TODO: Need to block signals or acquire some lock?!

  // TODO: Maybe do not use mmap to alloc, but instead some native allocation mechanism of L4Re?
  //       Because when creating many threads (e.g. in test_prio_list) this many
  //       mmap allocations overflow the reference counter of the
  //       counting_cap_alloc (only unsigned char with max val of 255) for the
  //       anon dataspace via alloc_anon_mem.
  unsigned char *map = __mmap(0, libc.tls_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
  if (map == MAP_FAILED)
    return NULL;

  pthread_descr new = __copy_tls(map);
  // NOTE: Same as in __init_tp...
  pthread_libc_data_t *new_libc_data = __pthread_descr_libc_data(new);
  new_libc_data->map_base = map;
  new_libc_data->map_size = libc.tls_size;
  new_libc_data->locale = &libc.global_locale;
  return ptlc_thread_descr_to_tls_tp(new);
}

void
ptlc_deallocate_tls(void *tcb)
{
  pthread_descr descr = ptlc_tls_tp_to_thread_descr(tcb);
  pthread_libc_data_t *libc_data = __pthread_descr_libc_data(descr);
  munmap(libc_data->map_base, libc_data->map_size);
}

void
ptlc_init_static_tls(void *arg)
{
  __init_tls(arg);
}

pthread_descr
ptlc_thread_descr_self(void)
{
  return ptlc_tls_tp_to_thread_descr((char *)__get_tp() - TP_OFFSET);
}

pthread_descr
ptlc_tls_tp_to_thread_descr(void *tls_tp)
{
#ifdef TLS_ABOVE_TP
  return (pthread_descr)((char *)tls_tp - __pthread_struct_size()); // TODO: What about aligment and stuff?!
#else
  return (void *)tls_tp;
#endif
}

void *
ptlc_thread_descr_to_tls_tp(pthread_descr descr)
{
#ifdef TLS_ABOVE_TP
  return (void *)((char *)descr + __pthread_struct_size()); // TODO: What about aligment and stuff?!
#else
  return (void *)descr;
#endif
}

void
ptlc_before_create_thread(void)
{
  __acquire_ptc();
}

void
ptlc_after_create_thread(bool success)
{
  if (success)
    __atomic_add_fetch(&libc.threads_minus_1, 1, __ATOMIC_SEQ_CST);

  __release_ptc();
}

void
ptlc_after_exit_thread()
{
  __atomic_sub_fetch(&libc.threads_minus_1, 1, __ATOMIC_SEQ_CST);
}
