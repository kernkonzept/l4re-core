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

#ifndef _DESCR_H
#define _DESCR_H	1

#define __need_res_state
#include <resolv.h>
#include <setjmp.h>
#include <stdint.h>
#include <sys/types.h>
#include <tls.h>
#include <l4/sys/utcb.h>
#include <pt-machine.h>
#include <sched.h>
/* The type of thread descriptors */
typedef struct pthread *pthread_descr;



/* Arguments passed to thread creation routine */
struct pthread_start_args {
  void *(*start_routine)(void *); /* function to run */
  void *arg;                      /* its argument */
};


/* Callback interface for removing the thread from waiting on an
   object if it is cancelled while waiting or about to wait.
   This hold a pointer to the object, and a pointer to a function
   which ``extricates'' the thread from its enqueued state.
   The function takes two arguments: pointer to the wait object,
   and a pointer to the thread. It returns 1 if an extrication
   actually occured, and hence the thread must also be signalled.
   It returns 0 if the thread had already been extricated. */
typedef struct _pthread_extricate_struct {
    void *pu_object;
    int (*pu_extricate_func)(void *, pthread_descr);
} pthread_extricate_if;


/* Atomic counter made possible by compare_and_swap */
struct pthread_atomic {
  long p_count;
  int p_spinlock;
};


/* Context info for read write locks. The pthread_rwlock_info structure
   is information about a lock that has been read-locked by the thread
   in whose list this structure appears. The pthread_rwlock_context
   is embedded in the thread context and contains a pointer to the
   head of the list of lock info structures, as well as a count of
   read locks that are untracked, because no info structure could be
   allocated for them. */
struct _pthread_rwlock_t;
typedef struct _pthread_rwlock_info {
  struct _pthread_rwlock_info *pr_next;
  struct _pthread_rwlock_t *pr_lock;
  int pr_lock_count;
} pthread_readlock_info;

#ifndef TCB_ALIGNMENT
# define TCB_ALIGNMENT	sizeof (double)
#endif


/* We keep thread specific data in a special data structure, a two-level
   array.  The top-level array contains pointers to dynamically allocated
   arrays of a certain number of data pointers.  So we can implement a
   sparse array.  Each dynamic second-level array has
        PTHREAD_KEY_2NDLEVEL_SIZE
   entries.  This value shouldn't be too large.  */
#define PTHREAD_KEY_2NDLEVEL_SIZE       32

/* We need to address PTHREAD_KEYS_MAX key with PTHREAD_KEY_2NDLEVEL_SIZE
   keys in each subarray.  */
#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
   / PTHREAD_KEY_2NDLEVEL_SIZE)


union dtv;

struct pthread
{
  union
  {
#if !defined(TLS_DTV_AT_TP)
    /* This overlaps the TCB as used for TLS without threads (see tls.h).  */
    tcbhead_t header;
#else
    struct
    {
      int multiple_threads;
      int gscope_flag;
# ifndef CONFIG_L4
# ifndef __ASSUME_PRIVATE_FUTEX
      int private_futex;
# endif
# endif
    } header;
#endif

    /* This extra padding has no special purpose, and this structure layout
       is private and subject to change without affecting the official ABI.
       We just have it here in case it might be convenient for some
       implementation-specific instrumentation hack or suchlike.  */
    void *__padding[24];
  };

  pthread_descr p_nextlive, p_prevlive;
                                /* Double chaining of active threads */
  pthread_descr p_nextwaiting;  /* Next element in the queue holding the thr */
  pthread_descr p_nextlock;	/* can be on a queue and waiting on a lock */
  pthread_t p_tid;              /* Thread identifier */

  int p_priority;               /* Thread priority (== 0 if not realtime) */
  int p_sched_policy;
  __cpu_mask p_affinity_mask[1]; /* L4 addition; small, more needs dynamic allocations */


  l4_cap_idx_t     p_thsem_cap;
  l4_cap_idx_t     p_th_cap;
  struct _pthread_fastlock * p_lock; /* Spinlock for synchronized accesses */
  sigjmp_buf * p_cancel_jmp;    /* where to siglongjmp on a cancel or NULL */
  char p_terminated;            /* true if terminated e.g. by pthread_exit */
  char p_detached;              /* true if detached */
  char p_exited;                /* true if the assoc. process terminated */
  void * p_retval;              /* placeholder for return value */
  int p_retcode;                /* placeholder for return code */
  pthread_descr p_joining;      /* thread joining on that thread or NULL */
  struct _pthread_cleanup_buffer * p_cleanup; /* cleanup functions */
  char p_cancelstate;           /* cancellation state */
  char p_canceltype;            /* cancellation type (deferred/async) */
  char p_canceled;              /* cancellation request pending */
  struct pthread_start_args p_start_args; /* arguments for thread creation */
  void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE]; /* thread-specific data */
#if !(USE_TLS && HAVE___THREAD)
  void * p_libc_specific[_LIBC_TSD_KEY_N]; /* thread-specific data for libc */
  int * p_errnop;               /* pointer to used errno variable */
  int p_errno;                  /* error returned by last system call */
  int * p_h_errnop;             /* pointer to used h_errno variable */
  int p_h_errno;                /* error returned by last netdb function */
  struct __res_state *p_resp;	/* Pointer to resolver state */
#endif
  struct __res_state p_res;	/* per-thread resolver state */
  int p_userstack;		/* nonzero if the user provided the stack */
  void *p_guardaddr;		/* address of guard area or NULL */
  size_t p_guardsize;		/* size of guard area */
  struct pthread_atomic p_resume_count; /* number of times restart() was
					   called on thread */
  char p_woken_by_cancel;       /* cancellation performed wakeup */
  char p_condvar_avail;		/* flag if conditional variable became avail */
  char p_sem_avail;             /* flag if semaphore became available */
  pthread_extricate_if *p_extricate; /* See above */
  pthread_readlock_info *p_readlock_list;  /* List of readlock info structs */
  pthread_readlock_info *p_readlock_free;  /* Free list of structs */
  int p_untracked_readlock_count;	/* Readlocks not tracked by list */
  int p_inheritsched;           /* copied from the thread attribute */
#ifdef USE_TLS
  char *p_stackaddr;		/* Stack address.  */
#endif
  size_t p_alloca_cutoff;	/* Maximum size which should be allocated
				   using alloca() instead of malloc().  */
  /* New elements must be added at the end.  */
} __attribute__ ((aligned (TCB_ALIGNMENT)));



/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary.
   Initially 0, meaning that the current thread is (by definition)
   the initial thread. */

extern char *__pthread_initial_thread_bos;

/* Descriptor of the initial thread */

extern struct pthread __pthread_initial_thread;

/* Limits of the thread manager stack. */

extern char *__pthread_manager_thread_bos;
extern char *__pthread_manager_thread_tos;

/* Descriptor of the manager thread */

extern struct pthread __pthread_manager_thread;
extern pthread_descr __pthread_manager_threadp L4_HIDDEN;

/* Indicate whether at least one thread has a user-defined stack (if 1),
   or all threads have stacks supplied by LinuxThreads (if 0). */

extern int __pthread_nonstandard_stacks;

/* The max size of the thread stack segments.  If the default
   THREAD_SELF implementation is used, this must be a power of two and
   a multiple of PAGE_SIZE.  */
#ifndef STACK_SIZE
#define STACK_SIZE  (2 * 1024 * 1024)
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#ifndef CURRENT_STACK_FRAME
#define CURRENT_STACK_FRAME  ({ char __csf; &__csf; })
#endif

/* Recover thread descriptor for the current thread */

extern pthread_descr __pthread_find_self (void) __attribute__ ((pure));

static __inline__ pthread_descr thread_self (void) __attribute__ ((pure));
static __inline__ pthread_descr thread_self (void)
{ return (pthread_descr)(l4_utcb_tcr()->user[0]); }

#endif	/* descr.h */
