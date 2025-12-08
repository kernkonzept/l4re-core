
/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Thread creation, initialization, and basic low-level routines */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <assert.h>

#include <l4/re/env.h>
#include <l4/sys/task.h>
#include <l4/util/util.h>
#include <l4/re/consts.h>

#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "smp.h"
#include <not-cancel.h>
#include <link.h>
#include <ldsodefs.h>

/* Sanity check.  */
#if !defined __SIGRTMIN || (__SIGRTMAX - __SIGRTMIN) < 3
# error "This must not happen"
#endif

/* mods for uClibc: __libc_sigaction is not in any standard headers */
extern __typeof(sigaction) __libc_sigaction;

/* We need only a few variables.  */
#define manager_thread __pthread_manager_threadp
pthread_descr __pthread_manager_threadp attribute_hidden;

/* Pointer to the main thread (the father of the thread manager thread) */
/* Originally, this is the initial thread, but this changes after fork() */

pthread_descr __pthread_main_thread;

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary. */

char *__pthread_initial_thread_bos;

/* File descriptor for sending requests to the thread manager. */
/* Initially -1, meaning that the thread manager is not running. */

l4_cap_idx_t __pthread_manager_request = L4_INVALID_CAP;

int __pthread_multiple_threads attribute_hidden;

/* Other end of the pipe for sending requests to the thread manager. */

int __pthread_manager_reader;

/* Limits of the thread manager stack */

char *__pthread_manager_thread_bos;
char *__pthread_manager_thread_tos;

/* For process-wide exit() */

int __pthread_exit_requested;
int __pthread_exit_code;

/* Maximum stack size.  */
size_t __pthread_max_stacksize;

/* Nozero if the machine has more than one processor.  */
int __pthread_smp_kernel;

/* Communicate relevant LinuxThreads constants to gdb */
const int __linuxthreads_pthread_sizeof_descr
  = sizeof(struct pthread);

const int __linuxthreads_initial_report_events;

const char __linuxthreads_version[] = VERSION;

/* Forward declarations */

static void pthread_onexit_process(int retcode, void *arg);
#ifndef HAVE_Z_NODELETE
static void pthread_atexit_process(void *arg, int retcode);
static void pthread_atexit_retcode(void *arg, int retcode);
#endif

extern int __libc_current_sigrtmin_private (void);

/* Initialize the pthread library.
   Initialization is split in two functions:
   - a constructor function that blocks the __pthread_sig_restart signal
     (must do this very early, since the program could capture the signal
      mask with e.g. sigsetjmp before creating the first thread);
   - a regular function called from pthread_create when needed. */

static void pthread_initialize(void) __attribute__((constructor));

#ifndef HAVE_Z_NODELETE
extern void *__dso_handle __attribute__ ((weak));
#endif


#if defined USE_TLS && !defined SHARED
extern void __libc_setup_tls (size_t tcbsize, size_t tcbalign);
#endif

static int *__libc_multiple_threads_ptr;
l4_utcb_t *__pthread_first_free_utcb attribute_hidden;

/*
 * Add the memory area [utcbs_start, utcbs_end] as chunks of L4_UTCB_OFFSET to
 * the UTCB free list.
 *
 * If (utcb_end - utcb_start >= L4_UTCB_OFFSET) then __pthread_first_free_utcb
 * points to at least one usable UTCB when the function returns.
 */
void
__l4_add_utcbs(l4_addr_t utcbs_start, l4_addr_t utcbs_end)
{
  l4_addr_t free_utcb = utcbs_start;
  l4_utcb_t **last_free = &__pthread_first_free_utcb;

  // __pthread_first_free_utcb is not necessarily NULL. There might be UTCBs on
  // the list which are currently not usable (See __l4_utcb_is_usable_now()).
  l4_utcb_t *old_first = __pthread_first_free_utcb;

  while (free_utcb + L4_UTCB_OFFSET <= utcbs_end)
    {
      l4_utcb_t *u = (l4_utcb_t*)free_utcb;
      l4_thread_regs_t *tcr = l4_utcb_tcr_u(u);
      tcr->user[0] = 0;
      __l4_utcb_set_next_free(u, 0);
      __l4_utcb_mark_unused(u);
      __pthread_init_lock(handle_to_lock(u));
      *last_free = u;
      last_free = (l4_utcb_t**)(&tcr->user[2]);
      free_utcb += L4_UTCB_OFFSET;
    }

  *last_free = old_first;
}

/* Do some minimal initialization which has to be done during the
   startup of the C library.  */
void
__pthread_initialize_minimal(void)
{
  static int initialized;
  if (initialized)
    return;

  initialized = 1;

  /* initialize free list */
  l4_fpage_t utcb_area = l4re_env()->utcb_area;
  l4_addr_t free_utcb = l4re_env()->first_free_utcb;
  l4_addr_t utcbs_end =
    l4_fpage_memaddr(utcb_area) + (1UL << (l4_addr_t)l4_fpage_size(utcb_area));
  __l4_add_utcbs(free_utcb, utcbs_end);
  /* All in the free pool now so indicate that first_free_utcb not available
   * anymore */
  l4re_env()->first_free_utcb = ~0UL;

  __pthread_init_lock(handle_to_lock(l4_utcb()));

  pthread_descr self;

  /* First of all init __pthread_handles[0] and [1] if needed.  */
#if __LT_SPINLOCK_INIT != 0
  __pthread_handles[0].h_lock = __LOCK_INITIALIZER;
  __pthread_handles[1].h_lock = __LOCK_INITIALIZER;
#endif
#ifndef SHARED
  /* Unlike in the dynamically linked case the dynamic linker has not
     taken care of initializing the TLS data structures.  */
  __libc_setup_tls (TLS_TCB_SIZE, TLS_TCB_ALIGN);
#elif !USE___THREAD
  if (__builtin_expect (GL(dl_tls_dtv_slotinfo_list) == NULL, 0))
    {
      tcbhead_t *tcbp;

      /* There is no actual TLS being used, so the thread register
	 was not initialized in the dynamic linker.  */

      /* We need to install special hooks so that the malloc and memalign
	 calls in _dl_tls_setup and _dl_allocate_tls won't cause full
	 malloc initialization that will try to set up its thread state.  */

      extern void __libc_malloc_pthread_startup (bool first_time);
      __libc_malloc_pthread_startup (true);

      if (__builtin_expect (_dl_tls_setup (), 0)
	  || __builtin_expect ((tcbp = _dl_allocate_tls (NULL)) == NULL, 0))
	{
	  static const char msg[] = "\
cannot allocate TLS data structures for initial thread\n";
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						msg, sizeof msg - 1));
	  abort ();
	}
      const char *lossage = TLS_INIT_TP (tcbp, 0);
      if (__builtin_expect (lossage != NULL, 0))
	{
	  static const char msg[] = "cannot set up thread-local storage: ";
	  const char nl = '\n';
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						msg, sizeof msg - 1));
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						lossage, strlen (lossage)));
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO, &nl, 1));
	}

      /* Though it was allocated with libc's malloc, that was done without
	 the user's __malloc_hook installed.  A later realloc that uses
	 the hooks might not work with that block from the plain malloc.
	 So we record this block as unfreeable just as the dynamic linker
	 does when it allocates the DTV before the libc malloc exists.  */
      GL(dl_initial_dtv) = GET_DTV (tcbp);

      __libc_malloc_pthread_startup (false);
    }
#endif

  self = THREAD_SELF;

  /* The memory for the thread descriptor was allocated elsewhere as
     part of the TLS allocation.  We have to initialize the data
     structure by hand.  This initialization must mirror the struct
     definition above.  */
  self->p_nextlive = self->p_prevlive = self;
  /* self->p_start_args need not be initialized, it's all zero.  */
  self->p_userstack = 1;
#if __LT_SPINLOCK_INIT != 0
  self->p_resume_count = (struct pthread_atomic) __ATOMIC_INITIALIZER;
#endif
  self->p_alloca_cutoff = __MAX_ALLOCA_CUTOFF;

  /* Another variable which points to the thread descriptor.  */
  __pthread_main_thread = self;

#if HP_TIMING_AVAIL
  self->p_cpuclock_offset = GL(dl_cpuclock_offset);
#endif
  if (__pthread_l4_initialize_main_thread(self))
    exit(1);

  __libc_multiple_threads_ptr = __libc_pthread_init ();
}


void
__pthread_init_max_stacksize(void)
{
  size_t max_stack;

  // L4
  max_stack = STACK_SIZE - L4_PAGESIZE;

  __pthread_max_stacksize = max_stack;
  if (max_stack / 4 < __MAX_ALLOCA_CUTOFF)
    {
      pthread_descr self = THREAD_SELF;
      self->p_alloca_cutoff = max_stack / 4;
    }
}

/* psm: we do not have any ld.so support yet
 *	 remove the USE_TLS guard if nptl is added */
#if defined SHARED && defined USE_TLS
# if USE___THREAD
/* When using __thread for this, we do it in libc so as not
   to give libpthread its own TLS segment just for this.  */
extern void **__libc_dl_error_tsd (void) __attribute__ ((const));
# else
static void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  return &thread_self ()->p_libc_specific[_LIBC_TSD_KEY_DL_ERROR];
}
# endif
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

static void pthread_initialize(void)
{
#ifdef NOT_USED
  struct sigaction sa;
  sigset_t mask;
#endif

  /* If already done (e.g. by a constructor called earlier!), bail out */
  if (__pthread_initial_thread_bos != NULL) return;
#ifdef TEST_FOR_COMPARE_AND_SWAP
  /* Test if compare-and-swap is available */
  __pthread_has_cas = compare_and_swap_is_available();
#endif
  /* We don't need to know the bottom of the stack.  Give the pointer some
     value to signal that initialization happened.  */
  __pthread_initial_thread_bos = (void *) -1l;
  /* Register an exit function to kill all other threads. */
  /* Do it early so that user-registered atexit functions are called
     before pthread_*exit_process. */
#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_process, NULL,
		  __dso_handle);
  else
#endif
    __on_exit (pthread_onexit_process, NULL);
  /* How many processors.  */
  __pthread_smp_kernel = is_smp_system ();

/* psm: we do not have any ld.so support yet
 *	 remove the USE_TLS guard if nptl is added */
#if defined SHARED && defined USE_TLS
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

void __pthread_initialize(void)
{
  pthread_initialize();
}

int __pthread_initialize_manager(void)
{
  pthread_descr mgr;
  tcbhead_t *tcbp;

  __pthread_multiple_threads = 1;
  __pthread_main_thread->header.multiple_threads = 1;
  *__libc_multiple_threads_ptr = 1;

#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_retcode, NULL,
		  __dso_handle);
#endif

  if (__pthread_max_stacksize == 0)
    __pthread_init_max_stacksize ();
  /* If basic initialization not done yet (e.g. we're called from a
     constructor run before our constructor), do it now */
  if (__pthread_initial_thread_bos == NULL) pthread_initialize();
  /* Setup stack for thread manager */
  __pthread_manager_thread_bos = malloc(THREAD_MANAGER_STACK_SIZE);
  if (__pthread_manager_thread_bos == NULL)
    return -1;
  __pthread_manager_thread_tos =
    __pthread_manager_thread_bos + THREAD_MANAGER_STACK_SIZE;
  // L4: force 16-byte stack alignment
  __pthread_manager_thread_tos =
    (char *)((uintptr_t)__pthread_manager_thread_tos & ~0xfUL);

  /* Allocate memory for the thread descriptor and the dtv.  */
  tcbp = _dl_allocate_tls (NULL);
  if (tcbp == NULL) {
    free(__pthread_manager_thread_bos);
    return -1;
  }

#if defined(TLS_TCB_AT_TP)
  mgr = (pthread_descr) tcbp;
#elif defined(TLS_DTV_AT_TP)
  /* pthread_descr is located right below tcbhead_t which _dl_allocate_tls
     returns.  */
  mgr = (pthread_descr) ((char *) tcbp - TLS_PRE_TCB_SIZE);
#endif

  /* Initialize the descriptor.  */
#if !TLS_DTV_AT_TP
  mgr->header.tcb = tcbp;
  mgr->header.self = mgr;
#endif
  mgr->header.multiple_threads = 1;
  mgr->p_start_args = (struct pthread_start_args) PTHREAD_START_ARGS_INITIALIZER(__pthread_manager);
#if __LT_SPINLOCK_INIT != 0
  self->p_resume_count = (struct pthread_atomic) __ATOMIC_INITIALIZER;
#endif
  mgr->p_alloca_cutoff = PTHREAD_STACK_MIN / 4;

  /* Start the thread manager */
  int err = __pthread_start_manager(mgr);

  if (__builtin_expect (err, 0) == -1) {
    _dl_deallocate_tls (tcbp, true);
    free(__pthread_manager_thread_bos);
    return -1;
  }

  return 0;
}

/* Thread creation */

int
attribute_hidden
__pthread_create(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  pthread_descr self = thread_self();
  struct pthread_request request;
  int retval;
  if (__builtin_expect (l4_is_invalid_cap(__pthread_manager_request), 0)) {
    if (__pthread_initialize_manager() < 0)
      return EAGAIN;
  }
  request.req_thread = self;
  request.req_kind = REQ_CREATE;
  request.req_args.create.attr = attr;
  request.req_args.create.fn = start_routine;
  request.req_args.create.arg = arg;
  __pthread_send_manager_rq(&request, 1);
  retval = THREAD_GETMEM(self, p_retcode);
  if (__builtin_expect (retval, 0) == 0)
    *thread = (pthread_t) THREAD_GETMEM(self, p_retval);
  return retval;
}
strong_alias (__pthread_create, pthread_create)

/* Simple operations on thread identifiers */

pthread_descr
attribute_hidden
__pthread_thread_self(void)
{
  return thread_self();
}

pthread_t
attribute_hidden
__pthread_self(void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_tid);
}
strong_alias (__pthread_self, pthread_self)

int
attribute_hidden
__pthread_equal(pthread_t thread1, pthread_t thread2)
{
  return thread1 == thread2;
}
strong_alias (__pthread_equal, pthread_equal)

/* Process-wide exit() request */

static void pthread_onexit_process(int retcode, void *arg)
{
  //l4/if (__builtin_expect (__pthread_manager_request, 0) >= 0) {
  if (!l4_is_invalid_cap(__pthread_manager_request)) {
    struct pthread_request request;
    pthread_descr self = thread_self();

    /* Make sure we come back here after suspend(), in case we entered
       from a signal handler.  */
    //l4/THREAD_SETMEM(self, p_signal_jmp, NULL);

    request.req_thread = self;
    request.req_kind = REQ_PROCESS_EXIT;
    request.req_args.exit.code = retcode;
    // let pthread-manager kill all pthreads except myself
    // exclude the main thread
    __pthread_send_manager_rq(&request, 1);
    // kill manager thread
    __l4_kill_thread(__pthread_manager_request);
    if (self != __pthread_main_thread)
      {
        // this was not called from the main thread, so kill it as well
        __l4_kill_thread(__pthread_main_thread->p_th_cap);
      }
    return;
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread)
      {
	UNIMPL();
	__pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
      }
  }
}

#ifndef HAVE_Z_NODELETE
static int __pthread_atexit_retcode;

static void pthread_atexit_process(void *arg, int retcode)
{
  pthread_onexit_process (retcode ?: __pthread_atexit_retcode, arg);
}

static void pthread_atexit_retcode(void *arg, int retcode)
{
  __pthread_atexit_retcode = retcode;
}
#endif

/* Concurrency symbol level.  */
static int current_level;

int
attribute_hidden
__pthread_setconcurrency(int level)
{
  /* We don't do anything unless we have found a useful interpretation.  */
  current_level = level;
  return 0;
}
weak_alias (__pthread_setconcurrency, pthread_setconcurrency)

int
attribute_hidden
__pthread_getconcurrency(void)
{
  return current_level;
}
weak_alias (__pthread_getconcurrency, pthread_getconcurrency)

/* trampoline function where threads are put before they are destroyed in
   __l4_kill_thread */
static void __l4_noop(void)
{
  l4_sleep_forever();
}

/*
 * Kill a thread hard.
 *
 * This function may only be used from pthreads exit handler. It kills a
 * thread hard. That means the thread does not get a chance to cleanup its
 * resources, including locks. We rely on the microkernel to free kernel
 * resources when the task object is destroyed.
 *
 * Ongoing IPC is canceled so that any locks the thread may hold in the
 * microkernel are freed.
 */
void __l4_kill_thread(l4_cap_idx_t cap)
{
  /* cancel any ongoing IPC and put the thread into the __l4_noop function */
  l4_thread_ex_regs(cap, (l4_addr_t)__l4_noop, ~0UL, L4_THREAD_EX_REGS_CANCEL);

  /* delete it */
  l4_task_delete_obj(L4RE_THIS_TASK_CAP, cap);
}


/* Debugging aid */

#ifdef DEBUG
#include <stdarg.h>

void
attribute_hidden
__pthread_message(const char * fmt, ...)
{
  char buffer[1024];
  va_list args;
  sprintf(buffer, "%05d : ", __getpid());
  va_start(args, fmt);
  vsnprintf(buffer + 8, sizeof(buffer) - 8, fmt, args);
  va_end(args);
  TEMP_FAILURE_RETRY(write_not_cancel(2, buffer, strlen(buffer)));
}

#endif
