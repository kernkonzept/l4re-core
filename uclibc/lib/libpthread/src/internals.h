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

#ifndef _INTERNALS_H
#define _INTERNALS_H	1

#include "uClibc-glue.h"

/* Internal data structures */

/* Includes */

#include <limits.h>
//l4/#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
//l4/#include <signal.h>
#include <bits/stackinfo.h>
//l4/#include <bits/sigcontextinfo.h>
#include <bits/pthreadtypes.h>

#include <bits/libc-lock.h>

#include <l4/sys/ipc.h>

#ifdef USE_TLS
#include <tls.h>
#endif
#include "descr.h"

#include "semaphore.h"
#include <pthread-functions.h>

#ifndef THREAD_GETMEM
# define THREAD_GETMEM(descr, member) descr->member
#endif
#ifndef THREAD_GETMEM_NC
# define THREAD_GETMEM_NC(descr, member, idx) descr->member[idx]
#endif
#ifndef THREAD_SETMEM
# define THREAD_SETMEM(descr, member, value) descr->member = (value)
#endif
#ifndef THREAD_SETMEM_NC
# define THREAD_SETMEM_NC(descr, member, idx, value) descr->member[idx] = (value)
#endif

#if !defined NOT_IN_libc
# define LIBC_THREAD_GETMEM(descr, member) THREAD_GETMEM (descr, member)
# define LIBC_THREAD_SETMEM(descr, member, value) \
  THREAD_SETMEM (descr, member, value)
#else
# define LIBC_THREAD_GETMEM(descr, member) descr->member
# define LIBC_THREAD_SETMEM(descr, member, value) descr->member = (value)
#endif

typedef void (*destr_function)(void *);

struct pthread_key_struct {
  int in_use;                   /* already allocated? */
  destr_function destr;         /* destruction routine */
};

#include <l4/sys/kdebug.h>
#define UNIMPL(x...) do { outstring("UNIMPL: " x "\n"); } while(0)


#define PTHREAD_START_ARGS_INITIALIZER(fct) \
  { (void *(*) (void *)) fct, NULL }


/* The type of thread handles. */
typedef l4_utcb_t *pthread_handle;


/* The type of messages sent to the thread manager thread */

enum pthread_request_rq {                        /* Request kind */
    REQ_CREATE, REQ_FREE, REQ_PROCESS_EXIT, REQ_MAIN_THREAD_EXIT,
    REQ_POST, REQ_DEBUG, REQ_KICK, REQ_FOR_EACH_THREAD,
    REQ_THREAD_EXIT
};

struct pthread_request {
  pthread_descr req_thread;     /* Thread doing the request */
  enum pthread_request_rq req_kind;
  union {                       /* Arguments for request */
    struct {                    /* For REQ_CREATE: */
      const pthread_attr_t * attr; /* thread attributes */
      void * (*fn)(void *);     /*   start function */
      void * arg;               /*   argument to start function */
    } create;
    struct {                    /* For REQ_FREE: */
      pthread_t thread_id;      /*   identifier of thread to free */
    } free;
    struct {                    /* For REQ_PROCESS_EXIT: */
      int code;                 /*   exit status */
    } exit;
    void * post;                /* For REQ_POST: the semaphore */
    struct {			/* For REQ_FOR_EACH_THREAD: callback */
      void (*fn)(void *, pthread_descr);
      void *arg;
    } for_each;
  } req_args;
};


/* First free thread */
extern l4_utcb_t *__pthread_first_free_handle attribute_hidden;

/* Descriptor of the main thread */

extern pthread_descr __pthread_main_thread;

/* File descriptor for sending requests to the thread manager.
   Initially -1, meaning that __pthread_initialize_manager must be called. */

extern l4_cap_idx_t __pthread_manager_request;

/* Other end of the pipe for sending requests to the thread manager. */

/* Maximum stack size.  */
extern size_t __pthread_max_stacksize;

/* Default stack size for new threads.  */
extern size_t __pthread_default_stacksize;

/* Pending request for a process-wide exit */

extern int __pthread_exit_requested, __pthread_exit_code;

/* Set to 1 by gdb if we're debugging */

extern __volatile__ int __pthread_threads_debug;

/* Globally enabled events.  */
//extern __volatile__ td_thr_events_t __pthread_threads_events;

/* Pointer to descriptor of thread with last event.  */
//extern __volatile__ pthread_descr __pthread_last_event;

/* Flag which tells whether we are executing on SMP kernel. */
extern int __pthread_smp_kernel;

inline static void __pthread_send_manager_rq(struct pthread_request *r, int block)
{
  if (l4_is_invalid_cap(__pthread_manager_request))
    return;
  __builtin_memcpy(l4_utcb_mr()->mr, r, sizeof(struct pthread_request));
  l4_msgtag_t tag
    = l4_msgtag(0,
                (sizeof(struct pthread_request) + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t),
                0, L4_MSGTAG_SCHEDULE);
  if (block)
    l4_ipc_call(__pthread_manager_request, l4_utcb(), tag, L4_IPC_NEVER);
  else
    l4_ipc_send(__pthread_manager_request, l4_utcb(), tag, L4_IPC_NEVER);
}

/* Return the handle corresponding to a thread id */

static __inline__ pthread_handle thread_handle(pthread_t id)
{
  return (l4_utcb_t*)id; //&__pthread_handles[id % PTHREAD_THREADS_MAX];
}

static inline pthread_descr handle_to_descr(pthread_handle h)
{ return (pthread_descr)(l4_utcb_tcr_u(h)->user[0]); }

static inline struct _pthread_fastlock *handle_to_lock(pthread_handle h)
{ return (struct _pthread_fastlock *)(&l4_utcb_tcr_u(h)->user[1]); }

/* Validate a thread handle. Must have acquired h->h_spinlock before. */

static __inline__ int invalid_handle(pthread_handle h, pthread_t id)
{
  return h != id || handle_to_descr(h) == NULL
    || handle_to_descr(h)->p_tid != id || handle_to_descr(h)->p_terminated;
}

static __inline__ int nonexisting_handle(pthread_handle h, pthread_t id)
{
  return handle_to_descr(h) == NULL || handle_to_descr(h)->p_tid != id;
}

/* Fill in defaults left unspecified by pt-machine.h.  */

/* We round up a value with page size. */
#ifndef page_roundup
#define page_roundup(v,p) ((((size_t) (v)) + (p) - 1) & ~((p) - 1))
#endif

/* The page size we can get from the system.  This should likely not be
   changed by the machine file but, you never know.  */
#ifndef PAGE_SIZE
#define PAGE_SIZE  (L4_PAGESIZE)
#endif

/* The initial size of the thread stack.  Must be a multiple of PAGE_SIZE.  */
#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE  (4 * PAGE_SIZE)
#endif

/* Size of the thread manager stack. The "- 32" avoids wasting space
   with some malloc() implementations. */
#ifndef THREAD_MANAGER_STACK_SIZE
#define THREAD_MANAGER_STACK_SIZE  (2 * PAGE_SIZE - 32)
#endif

/* The base of the "array" of thread stacks.  The array will grow down from
   here.  Defaults to the calculated bottom of the initial application
   stack.  */
#ifndef THREAD_STACK_START_ADDRESS
#define THREAD_STACK_START_ADDRESS  __pthread_initial_thread_bos
#endif

/* If MEMORY_BARRIER isn't defined in pt-machine.h, assume the
   architecture doesn't need a memory barrier instruction (e.g. Intel
   x86).  Still we need the compiler to respect the barrier and emit
   all outstanding operations which modify memory.  Some architectures
   distinguish between full, read and write barriers.  */

#ifndef MEMORY_BARRIER
#define MEMORY_BARRIER() __asm__ ("" : : : "memory")
#endif
#ifndef READ_MEMORY_BARRIER
#define READ_MEMORY_BARRIER() MEMORY_BARRIER()
#endif
#ifndef WRITE_MEMORY_BARRIER
#define WRITE_MEMORY_BARRIER() MEMORY_BARRIER()
#endif

/* Max number of times we must spin on a spinlock calling sched_yield().
   After MAX_SPIN_COUNT iterations, we put the calling thread to sleep. */

#ifndef MAX_SPIN_COUNT
#define MAX_SPIN_COUNT 50
#endif

/* Max number of times the spinlock in the adaptive mutex implementation
   spins actively on SMP systems.  */

#ifndef MAX_ADAPTIVE_SPIN_COUNT
#define MAX_ADAPTIVE_SPIN_COUNT 100
#endif

/* Duration of sleep (in nanoseconds) when we can't acquire a spinlock
   after MAX_SPIN_COUNT iterations of sched_yield().
   With the 2.0 and 2.1 kernels, this MUST BE > 2ms.
   (Otherwise the kernel does busy-waiting for realtime threads,
    giving other threads no chance to run.) */

#ifndef SPIN_SLEEP_DURATION
#define SPIN_SLEEP_DURATION 2000001
#endif

/* Defined and used in libc.so.  */
extern int __libc_multiple_threads L4_HIDDEN;
extern int __librt_multiple_threads;

/* Debugging */

#ifdef DEBUG
#include <assert.h>
#define ASSERT assert
#define MSG __pthread_message
#else
#define ASSERT(x)
#define MSG(msg,arg...)
#endif

# define INIT_THREAD_SELF(descr, nr) do { l4_utcb_tcr()->user[0] = (l4_umword_t)descr; } while (0)



/* Internal global functions */
__BEGIN_DECLS
extern int __pthread_l4_initialize_main_thread(pthread_descr th) attribute_hidden;
extern void __l4_add_utcbs(l4_addr_t start, l4_addr_t utcbs_end);


extern int __pthread_sched_idle_prio;
extern int __pthread_sched_other_prio;
extern int __pthread_sched_rr_prio_min;
extern int __pthread_sched_rr_prio_max;


extern void __pthread_cleanup_push(struct _pthread_cleanup_buffer * buffer,
			   void (*routine)(void *), void * arg);
extern void __pthread_cleanup_pop(struct _pthread_cleanup_buffer * buffer,
			  int execute);
extern void __pthread_cleanup_pop_restore(struct _pthread_cleanup_buffer * buffer,
				  int execute);
extern void __pthread_cleanup_push_defer(struct _pthread_cleanup_buffer * buffer,
				 void (*routine)(void *), void * arg);

extern void __pthread_do_exit (void *retval, char *currentframe)
     __attribute__ ((__noreturn__));
extern void __pthread_destroy_specifics (void);
extern void __pthread_perform_cleanup (char *currentframe) internal_function;
extern void __pthread_init_max_stacksize (void);
extern int __pthread_initialize_manager (void);
extern void __pthread_message (const char * fmt, ...);
extern int __pthread_manager (void *reqfd);
extern int __pthread_start_manager (pthread_descr mgr) L4_HIDDEN;
extern int __pthread_manager_event (void *reqfd);
extern void __pthread_manager_sighandler (int sig);
extern void __pthread_reset_main_thread (void);
extern void __pthread_once_fork_prepare (void);
extern void __pthread_once_fork_parent (void);
extern void __pthread_once_fork_child (void);
extern void __flockfilelist (void);
extern void __funlockfilelist (void);
extern void __fresetlockfiles (void);
extern void __pthread_manager_adjust_prio (int thread_prio);
extern void __pthread_initialize_minimal (void);

extern int __pthread_attr_setguardsize (pthread_attr_t *__attr,
					size_t __guardsize);
extern int __pthread_attr_getguardsize (const pthread_attr_t *__attr,
					size_t *__guardsize);
#if 0 /* uClibc: deprecated stuff disabled */
extern int __pthread_attr_setstackaddr (pthread_attr_t *__attr,
					void *__stackaddr);
extern int __pthread_attr_getstackaddr (const pthread_attr_t *__attr,
					void **__stackaddr);
#endif
extern int __pthread_attr_setstacksize (pthread_attr_t *__attr,
					size_t __stacksize);
extern int __pthread_attr_getstacksize (const pthread_attr_t *__attr,
					size_t *__stacksize);
extern int __pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
				    size_t __stacksize);
extern int __pthread_attr_getstack (const pthread_attr_t *__attr, void **__stackaddr,
				    size_t *__stacksize);
extern int __pthread_attr_destroy (pthread_attr_t *attr);
extern int __pthread_attr_setdetachstate (pthread_attr_t *attr,
					  int detachstate);
extern int __pthread_attr_getdetachstate (const pthread_attr_t *attr,
					  int *detachstate);
extern int __pthread_attr_setschedparam (pthread_attr_t *attr,
					 const struct sched_param *param);
extern int __pthread_attr_getschedparam (const pthread_attr_t *attr,
					 struct sched_param *param);
extern int __pthread_attr_setschedpolicy (pthread_attr_t *attr, int policy);
extern int __pthread_attr_getschedpolicy (const pthread_attr_t *attr,
					  int *policy);
extern int __pthread_attr_setinheritsched (pthread_attr_t *attr, int inherit);
extern int __pthread_attr_getinheritsched (const pthread_attr_t *attr,
					   int *inherit);
extern int __pthread_attr_setscope (pthread_attr_t *attr, int scope);
extern int __pthread_attr_getscope (const pthread_attr_t *attr, int *scope);

extern int __pthread_getconcurrency (void);
extern int __pthread_setconcurrency (int __level);
extern int __pthread_mutex_timedlock (pthread_mutex_t *__mutex,
				      const struct timespec *__abstime);
extern int __pthread_mutexattr_getpshared (const pthread_mutexattr_t *__attr,
					   int *__pshared);
extern int __pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
					   int __pshared);
extern int __pthread_mutexattr_gettype (const pthread_mutexattr_t *__attr,
					int *__kind);
extern void __pthread_kill_other_threads_np (void);
extern int __pthread_mutex_init (pthread_mutex_t *__mutex,
				 __const pthread_mutexattr_t *__mutex_attr);
extern int __pthread_mutex_destroy (pthread_mutex_t *__mutex);
extern int __pthread_mutex_lock (pthread_mutex_t *__mutex);
extern int __pthread_mutex_trylock (pthread_mutex_t *__mutex);
extern int __pthread_mutex_unlock (pthread_mutex_t *__mutex);

extern int __pthread_cond_init (pthread_cond_t *cond,
				const pthread_condattr_t *cond_attr);
extern int __pthread_cond_destroy (pthread_cond_t *cond);
extern int __pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex);
extern int __pthread_cond_timedwait (pthread_cond_t *cond,
				     pthread_mutex_t *mutex,
				     const struct timespec *abstime);
extern int __pthread_cond_signal (pthread_cond_t *cond);
extern int __pthread_cond_broadcast (pthread_cond_t *cond);
extern int __pthread_condattr_init (pthread_condattr_t *attr);
extern int __pthread_condattr_destroy (pthread_condattr_t *attr);
extern pthread_t __pthread_self (void);
extern pthread_descr __pthread_thread_self (void);
extern pthread_descr __pthread_self_stack (void) L4_HIDDEN;
extern int __pthread_equal (pthread_t thread1, pthread_t thread2);
extern void __pthread_exit (void *retval)
#if defined NOT_IN_libc && defined IS_IN_libpthread
	attribute_noreturn
#endif
	;
extern int __pthread_getschedparam (pthread_t thread, int *policy,
				    struct sched_param *param) __THROW;
extern int __pthread_setschedparam (pthread_t thread, int policy,
				    const struct sched_param *param) __THROW;
extern int __pthread_setcancelstate (int state, int * oldstate);
extern int __pthread_setcanceltype (int type, int * oldtype);

extern int __pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
					 __const struct timespec *__restrict
					 __abstime);
extern int __pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
					 __const struct timespec *__restrict
					 __abstime);
extern int __pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr);

extern int __pthread_barrierattr_getpshared (__const pthread_barrierattr_t *
					     __restrict __attr,
					     int *__restrict __pshared);

extern int __pthread_spin_lock (pthread_spinlock_t *__lock);
extern int __pthread_spin_trylock (pthread_spinlock_t *__lock);
extern int __pthread_spin_unlock (pthread_spinlock_t *__lock);
extern int __pthread_spin_init (pthread_spinlock_t *__lock, int __pshared);
extern int __pthread_spin_destroy (pthread_spinlock_t *__lock);

/* Global pointers to old or new suspend functions */

extern void (*__pthread_restart)(pthread_descr);
extern void (*__pthread_suspend)(pthread_descr);
extern int (*__pthread_timedsuspend)(pthread_descr, const struct timespec *);

/* Prototypes for some of the new semaphore functions.  */
extern int sem_post (sem_t * sem) __THROW;
extern int sem_init (sem_t *__sem, int __pshared, unsigned int __value) __THROW;
extern int sem_wait (sem_t *__sem);
extern int sem_trywait (sem_t *__sem) __THROW;
extern int sem_getvalue (sem_t *__restrict __sem, int *__restrict __sval) __THROW;
extern int sem_destroy (sem_t *__sem) __THROW;

/* Prototypes for compatibility functions.  */
extern int __pthread_attr_init (pthread_attr_t *__attr);
extern int __pthread_create (pthread_t *__restrict __threadp,
				 const pthread_attr_t *__attr,
				 void *(*__start_routine) (void *),
				 void *__restrict __arg);

/* The functions called the signal events.  */
extern void __linuxthreads_create_event (void);
extern void __linuxthreads_death_event (void);
extern void __linuxthreads_reap_event (void);

/* This function is called to initialize the pthread library.  */
extern void __pthread_initialize (void);

/* TSD.  */
extern int __pthread_internal_tsd_set (int key, const void * pointer);
extern void * __pthread_internal_tsd_get (int key);
extern void ** __attribute__ ((__const__))
  __pthread_internal_tsd_address (int key);
#if 0
/* Sighandler wrappers.  */
extern void __pthread_sighandler(int signo, SIGCONTEXT ctx);
extern void __pthread_sighandler_rt(int signo, struct siginfo *si,
				    struct ucontext *uc);
extern void __pthread_null_sighandler(int sig);
extern int __pthread_sigaction (int sig, const struct sigaction *act,
				struct sigaction *oact);
extern int __pthread_sigwait (const sigset_t *set, int *sig);
extern int __pthread_raise (int sig);
#endif
/* Cancellation.  */
extern int __pthread_enable_asynccancel (void) L4_HIDDEN;
extern void __pthread_disable_asynccancel (int oldtype)
  internal_function L4_HIDDEN;

/* The two functions are in libc.so and not exported.  */
extern int __libc_enable_asynccancel (void) L4_HIDDEN;
extern void __libc_disable_asynccancel (int oldtype)
  internal_function L4_HIDDEN;

/* The two functions are in libc.so and are exported.  */
extern int __librt_enable_asynccancel (void);
extern void __librt_disable_asynccancel (int oldtype) internal_function;

extern void __pthread_cleanup_upto (__jmp_buf target,
				    char *targetframe) L4_HIDDEN;
#if 0
extern pid_t __pthread_fork (struct fork_block *b) L4_HIDDEN;
#endif

#define asm_handle(name) _asm_handle(name)
#define _asm_handle(name) #name
#define ASM_GLOBAL asm_handle(ASM_GLOBAL_DIRECTIVE)
#define ASM_CANCEL(name) asm_handle(C_SYMBOL_NAME(name))

#if !defined NOT_IN_libc
# define LIBC_CANCEL_ASYNC() \
  __libc_enable_asynccancel ()
# define LIBC_CANCEL_RESET(oldtype) \
  __libc_disable_asynccancel (oldtype)
# define LIBC_CANCEL_HANDLED() \
  __asm__ (ASM_GLOBAL " " ASM_CANCEL(__libc_enable_asynccancel)); \
  __asm__ (ASM_GLOBAL " " ASM_CANCEL(__libc_disable_asynccancel))
#elif defined IS_IN_libpthread
# define LIBC_CANCEL_ASYNC() \
  __pthread_enable_asynccancel ()
# define LIBC_CANCEL_RESET(oldtype) \
  __pthread_disable_asynccancel (oldtype)
# define LIBC_CANCEL_HANDLED() \
  __asm__ (ASM_GLOBAL " " ASM_CANCEL(__pthread_enable_asynccancel)); \
  __asm__ (ASM_GLOBAL " " ASM_CANCEL(__pthread_disable_asynccancel))
#elif defined IS_IN_librt
# define LIBC_CANCEL_ASYNC() \
  __librt_enable_asynccancel ()
# define LIBC_CANCEL_RESET(oldtype) \
  __librt_disable_asynccancel (oldtype)
# define LIBC_CANCEL_HANDLED() \
  __asm__ (ASM_GLOBAL " " ASM_CANCEL(__librt_enable_asynccancel)); \
  __asm__ (ASM_GLOBAL " " ASM_CANCEL(__librt_disable_asynccancel))
#else
# define LIBC_CANCEL_ASYNC()    0 /* Just a dummy value.  */
# define LIBC_CANCEL_RESET(val) ((void)(val)) /* Nothing, but evaluate it.  */
# define LIBC_CANCEL_HANDLED()	/* Nothing.  */
#endif
extern int * __libc_pthread_init (const struct pthread_functions *functions);
__END_DECLS


#ifndef USE_TLS
# define __manager_thread (&__pthread_manager_thread)
#else
# define __manager_thread __pthread_manager_threadp
#endif


static inline int __pthread_getprio(int policy, int prio);
static inline int __pthread_getprio(int policy, int prio)
{
    switch (policy)
    {
    case SCHED_OTHER:
      prio = 0;
      break;
    case SCHED_IDLE:
      prio = 0;
      break;
    case SCHED_RR:
      prio -= __pthread_sched_rr_prio_min;
      break;
    case SCHED_L4:
      break;
    default:
      return -1;
    }

    return prio;
}

static inline int __pthread_l4_getprio(int policy, int posix_prio);
static inline int __pthread_l4_getprio(int policy, int posix_prio)
{
  switch (policy)
    {
    case SCHED_OTHER:
      return __pthread_sched_other_prio;
    case SCHED_IDLE:
      return __pthread_sched_idle_prio;
    case SCHED_RR:
      return __pthread_sched_rr_prio_min + posix_prio;
    case SCHED_L4:
      return posix_prio;
    default:
      return -1;
    }
}


static inline pthread_descr
check_thread_self (void);
static inline pthread_descr
check_thread_self (void)
{
  pthread_descr self = thread_self ();
#if defined THREAD_SELF && defined INIT_THREAD_SELF
  if (self == __manager_thread)
    {
      /* A new thread might get a cancel signal before it is fully
	 initialized, so that the thread register might still point to the
	 manager thread.  Double check that this is really the manager
	 thread.  */
      self = handle_to_descr(l4_utcb());
      if (self != __manager_thread)
	/* Oops, thread_self() isn't working yet..  */
	INIT_THREAD_SELF(self, self->p_nr);
    }
#endif
  return self;
}

#endif /* internals.h */
