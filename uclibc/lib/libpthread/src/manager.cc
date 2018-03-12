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

/* The "thread manager" thread: manages creation and termination of threads */

#ifndef PT_EI
#define PT_EI inline
#endif

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <locale.h>		/* for __uselocale */

#include <l4/sys/ipc.h>
#include <l4/re/env>
#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/rm>
#include <l4/re/util/cap_alloc>
#include <l4/sys/capability>
#include <l4/sys/factory>
#include <l4/sys/scheduler>
#include <l4/sys/thread>

extern "C" {
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "semaphore.h"
#include "l4.h"
#include <ldsodefs.h>
}

#include <pthread-l4.h>

#define USE_L4RE_FOR_STACK

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

extern "C" void __pthread_new_thread_entry(void);

#ifndef THREAD_SELF
/* Indicate whether at least one thread has a user-defined stack (if 1),
   or if all threads have stacks supplied by LinuxThreads (if 0). */
int __pthread_nonstandard_stacks;
#endif

/* Number of active entries in __pthread_handles (used by gdb) */
__volatile__ int __pthread_handles_num = 2;

/* Whether to use debugger additional actions for thread creation
   (set to 1 by gdb) */
__volatile__ int __pthread_threads_debug;

static pthread_descr manager_thread;

/* Mapping from stack segment to thread descriptor. */
/* Stack segment numbers are also indices into the __pthread_handles array. */
/* Stack segment number 0 is reserved for the initial thread. */

# define thread_segment(seq) NULL

/* Flag set in signal handler to record child termination */

static __volatile__ int terminated_children;

/* Flag set when the initial thread is blocked on pthread_exit waiting
   for all other threads to terminate */

static int main_thread_exiting;

/* Counter used to generate unique thread identifier.
   Thread identifier is pthread_threads_counter + segment. */

//l4/static pthread_t pthread_threads_counter;

/* Forward declarations */

static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
                                 void * (*start_routine)(void *), void *arg);
static void pthread_handle_free(pthread_t th_id);
static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
     __attribute__ ((noreturn));
//l4/static void pthread_kill_all_threads(int main_thread_also);
static void pthread_for_each_thread(void *arg,
    void (*fn)(void *, pthread_descr));

static int pthread_exited(pthread_descr th);

/* The server thread managing requests for thread creation and termination */

int
__attribute__ ((noreturn))
__pthread_manager(void *arg)
{
  pthread_descr self = manager_thread = (pthread_descr)arg;
  struct pthread_request request;

#ifdef USE_TLS
# if defined(TLS_TCB_AT_TP)
  TLS_INIT_TP(self, 0);
#elif defined(TLS_DTV_AT_TP)
  TLS_INIT_TP((char *)self + TLS_PRE_TCB_SIZE, 0);
#else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
#endif
  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, 1);
#endif
#if !(USE_TLS && HAVE___THREAD)
  /* Set the error variable.  */
  self->p_errnop = &self->p_errno;
  self->p_h_errnop = &self->p_h_errno;
#endif
  /* Raise our priority to match that of main thread */
  __pthread_manager_adjust_prio(__pthread_main_thread->p_priority);

  l4_umword_t src;
  l4_msgtag_t tag = l4_msgtag(0, 0, 0, L4_MSGTAG_SCHEDULE);
  int do_reply = 0;
  /* Enter server loop */
  while (1)
    {
      if (do_reply)
	tag = l4_ipc_reply_and_wait(l4_utcb(), tag, &src, L4_IPC_NEVER);
      else
	tag = l4_ipc_wait(l4_utcb(), &src, L4_IPC_NEVER);

      if (l4_msgtag_has_error(tag))
	{
	  do_reply = 0;
	  continue;
	}

      memcpy(&request, l4_utcb_mr()->mr, sizeof(request));

      do_reply = 0;
      switch(request.req_kind)
	{
	case REQ_CREATE:
	  request.req_thread->p_retcode =
	    pthread_handle_create((pthread_t *) &request.req_thread->p_retval,
		request.req_args.create.attr,
		request.req_args.create.fn,
		request.req_args.create.arg);
	  do_reply = 1;
	  break;
	case REQ_FREE:
	  pthread_handle_free(request.req_args.free.thread_id);
	  break;
	case REQ_PROCESS_EXIT:
	  pthread_handle_exit(request.req_thread,
	      request.req_args.exit.code);
	  /* NOTREACHED */
	  break;
	case REQ_MAIN_THREAD_EXIT:
	  main_thread_exiting = 1;
          /* Reap children in case all other threads died and the signal handler
             went off before we set main_thread_exiting to 1, and therefore did
             not do REQ_KICK. */
          //l4/pthread_reap_children();

	  if (__pthread_main_thread->p_nextlive == __pthread_main_thread) {
	      restart(__pthread_main_thread);
	      /* The main thread will now call exit() which will trigger an
		 __on_exit handler, which in turn will send REQ_PROCESS_EXIT
		 to the thread manager. In case you are wondering how the
		 manager terminates from its loop here. */
	  }
	  break;
	case REQ_POST:
	  sem_post((sem_t*)request.req_args.post);
	  break;
	case REQ_DEBUG:
#ifdef NOT_FOR_L4
	  /* Make gdb aware of new thread and gdb will restart the
	     new thread when it is ready to handle the new thread. */
	  if (__pthread_threads_debug && __pthread_sig_debug > 0)
	    raise(__pthread_sig_debug);
#else
	  do_reply = 1;
#endif
	  break;
	case REQ_KICK:
	  /* This is just a prod to get the manager to reap some
	     threads right away, avoiding a potential delay at shutdown. */
	  break;
	case REQ_FOR_EACH_THREAD:
	  pthread_for_each_thread(request.req_args.for_each.arg,
	      request.req_args.for_each.fn);
          restart(request.req_thread);
	  do_reply = 1;
	  break;
        case REQ_THREAD_EXIT:
            {
              if (!pthread_exited(request.req_thread))
                {
                  auto th = request.req_thread;
                  /* Thread still waiting to be joined. Only release
                     L4 resources for now. */
                  using L4Re::Util::Auto_cap;
                  Auto_cap<void>::Cap s = L4::Cap<void>(th->p_thsem_cap);
                  th->p_thsem_cap = L4_INVALID_CAP;
                  Auto_cap<void>::Cap t = L4::Cap<void>(th->p_th_cap);
                  th->p_th_cap = L4_INVALID_CAP;
                }
            }
          break;
	}
      tag = l4_msgtag(0, 0, 0, L4_MSGTAG_SCHEDULE);
    }
}

int __pthread_manager_event(void *arg)
{
  pthread_descr self = (pthread_descr)arg;
  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, 1);
#endif

  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM(self, p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM(self, p_lock));

  return __pthread_manager(arg);
}

/* Process creation */

static int
__attribute__ ((noreturn))
pthread_start_thread(void *arg)
{
  pthread_descr self = (pthread_descr) arg;
#ifdef USE_TLS
# if defined(TLS_TCB_AT_TP)
  TLS_INIT_TP(self, 0);
#elif defined(TLS_DTV_AT_TP)
  TLS_INIT_TP((char *)self + TLS_PRE_TCB_SIZE, 0);
#else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
#endif

#ifdef NOT_FOR_L4
  struct pthread_request request;
#endif
  void * outcome;
#if HP_TIMING_AVAIL
  hp_timing_t tmpclock;
#endif
  /* Initialize special thread_self processing, if any.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
#if HP_TIMING_AVAIL
  HP_TIMING_NOW (tmpclock);
  THREAD_SETMEM (self, p_cpuclock_offset, tmpclock);
#endif

#ifdef NOT_FOR_L4
  /* Set the scheduling policy and priority for the new thread, if needed */
  if (THREAD_GETMEM(self, p_start_args.schedpolicy) >= 0)
    /* Explicit scheduling attributes were provided: apply them */
    __sched_setscheduler(THREAD_GETMEM(self, p_pid),
			 THREAD_GETMEM(self, p_start_args.schedpolicy),
                         &self->p_start_args.schedparam);
  else if (manager_thread->p_priority > 0)
    /* Default scheduling required, but thread manager runs in realtime
       scheduling: switch new thread to SCHED_OTHER policy */
    {
      struct sched_param default_params;
      default_params.sched_priority = 0;
      __sched_setscheduler(THREAD_GETMEM(self, p_pid),
                           SCHED_OTHER, &default_params);
    }
#if !(USE_TLS && HAVE___THREAD)
  /* Initialize thread-locale current locale to point to the global one.
     With __thread support, the variable's initializer takes care of this.  */
  __uselocale (LC_GLOBAL_LOCALE);
#else
  /* Initialize __resp.  */
  __resp = &self->p_res;
#endif
  /* Make gdb aware of new thread */
  if (__pthread_threads_debug && __pthread_sig_debug > 0) {
    request.req_thread = self;
    request.req_kind = REQ_DEBUG;
    TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
					(char *) &request, sizeof(request)));
    suspend(self);
  }
#endif
  /* Run the thread code */
  outcome = self->p_start_args.start_routine(THREAD_GETMEM(self,
							   p_start_args.arg));
  /* Exit with the given return value */
  __pthread_do_exit(outcome, (char *)CURRENT_STACK_FRAME);
}

#ifdef NOT_FOR_L4
static int
__attribute__ ((noreturn))
pthread_start_thread_event(void *arg)
{
  pthread_descr self = (pthread_descr) arg;

#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM(self, p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM(self, p_lock));

  /* Continue with the real function.  */
  pthread_start_thread (arg);
}
#endif

#ifdef USE_L4RE_FOR_STACK
static int pthread_l4_free_stack(void *stack_addr, void *guardaddr)
{
  L4Re::Env const *e = L4Re::Env::env();
  int err;
  L4::Cap<L4Re::Dataspace> ds;

  err = e->rm()->detach(stack_addr, &ds);
  if (err < 0)
    return err;

  L4Re::Util::cap_alloc.free(ds);

  return e->rm()->free_area((l4_addr_t)guardaddr);
}
#endif

static int pthread_allocate_stack(const pthread_attr_t *attr,
                                  pthread_descr default_new_thread,
                                  int pagesize,
                                  char ** out_new_thread,
                                  char ** out_new_thread_bottom,
                                  char ** out_guardaddr,
                                  size_t * out_guardsize,
                                  size_t * out_stacksize)
{
  pthread_descr new_thread;
  char * new_thread_bottom;
  char * guardaddr;
  size_t stacksize, guardsize;

#ifdef USE_TLS
  /* TLS cannot work with fixed thread descriptor addresses.  */
  assert (default_new_thread == NULL);
#endif

  if (attr != NULL && attr->__stackaddr_set)
    {
#ifdef _STACK_GROWS_UP
      /* The user provided a stack. */
# ifdef USE_TLS
      /* This value is not needed.  */
      new_thread = (pthread_descr) attr->__stackaddr;
      new_thread_bottom = (char *) new_thread;
# else
      new_thread = (pthread_descr) attr->__stackaddr;
      new_thread_bottom = (char *) (new_thread + 1);
# endif
      guardaddr = attr->__stackaddr + attr->__stacksize;
      guardsize = 0;
#else
      /* The user provided a stack.  For now we interpret the supplied
	 address as 1 + the highest addr. in the stack segment.  If a
	 separate register stack is needed, we place it at the low end
	 of the segment, relying on the associated stacksize to
	 determine the low end of the segment.  This differs from many
	 (but not all) other pthreads implementations.  The intent is
	 that on machines with a single stack growing toward higher
	 addresses, stackaddr would be the lowest address in the stack
	 segment, so that it is consistently close to the initial sp
	 value. */
# ifdef USE_TLS
      new_thread = (pthread_descr) attr->__stackaddr;
# else
      new_thread =
        (pthread_descr) ((long)(attr->__stackaddr) & -sizeof(void *)) - 1;
# endif
      new_thread_bottom = (char *) attr->__stackaddr - attr->__stacksize;
      guardaddr = new_thread_bottom;
      guardsize = 0;
#endif
#ifndef THREAD_SELF
      __pthread_nonstandard_stacks = 1;
#endif
#ifndef USE_TLS
      /* Clear the thread data structure.  */
      memset (new_thread, '\0', sizeof (*new_thread));
#endif
      stacksize = attr->__stacksize;
    }
  else
    {
      const size_t granularity = pagesize;
      void *map_addr;

      /* Allocate space for stack and thread descriptor at default address */
      if (attr != NULL)
	{
	  guardsize = page_roundup (attr->__guardsize, granularity);
	  stacksize = __pthread_max_stacksize - guardsize;
	  stacksize = MIN (stacksize,
			   page_roundup (attr->__stacksize, granularity));
	}
      else
	{
	  guardsize = granularity;
	  stacksize = __pthread_max_stacksize - guardsize;
	}

#ifdef USE_L4RE_FOR_STACK
      map_addr = 0;
      L4Re::Env const *e = L4Re::Env::env();
      long err;

      if (e->rm()->reserve_area(&map_addr, stacksize + guardsize,
	                        L4Re::Rm::Search_addr) < 0)
	return -1;

      guardaddr = (char*)map_addr;

      L4::Cap<L4Re::Dataspace> ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
      if (!ds.is_valid())
	return -1;

      err = e->mem_alloc()->alloc(stacksize, ds);

      if (err < 0)
	{
	  L4Re::Util::cap_alloc.free(ds);
	  e->rm()->free_area(l4_addr_t(map_addr));
	  return -1;
	}

      new_thread_bottom = (char *) map_addr + guardsize;
      err = e->rm()->attach(&new_thread_bottom, stacksize, L4Re::Rm::In_area,
                            L4::Ipc::make_cap_rw(ds), 0);

      if (err < 0)
	{
	  L4Re::Util::cap_alloc.free(ds);
	  e->rm()->free_area(l4_addr_t(map_addr));
	  return -1;
	}
#else
      map_addr = mmap(NULL, stacksize + guardsize,
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (map_addr == MAP_FAILED)
        /* No more memory available.  */
        return -1;

      guardaddr = (char *)map_addr;
      if (guardsize > 0)
        mprotect (guardaddr, guardsize, PROT_NONE);

      new_thread_bottom = (char *) map_addr + guardsize;
#endif

#ifdef USE_TLS
      new_thread = ((pthread_descr) (new_thread_bottom + stacksize));
#else
      new_thread = ((pthread_descr) (new_thread_bottom + stacksize)) - 1;
#endif
    }
  *out_new_thread = (char *) new_thread;
  *out_new_thread_bottom = new_thread_bottom;
  *out_guardaddr = guardaddr;
  *out_guardsize = guardsize;
  *out_stacksize = stacksize;
  return 0;
}

static inline
int __pthread_mgr_create_thread(pthread_descr thread, char **tos,
                                int (*f)(void*), int prio,
                                unsigned create_flags,
                                l4_sched_cpu_set_t const &affinity)
{
  using namespace L4Re;
  Env const *e = Env::env();
  L4Re::Util::Auto_cap<L4::Thread>::Cap _t = L4Re::Util::cap_alloc.alloc<L4::Thread>();
  if (!_t.is_valid())
    return ENOMEM;

  L4Re::Util::Auto_cap<Th_sem_cap>::Cap th_sem
    =  L4Re::Util::cap_alloc.alloc<Th_sem_cap>();
  if (!th_sem.is_valid())
    return ENOMEM;

  int err = l4_error(e->factory()->create_thread(_t.get()));
  if (err < 0)
    return -err;

  // needed by __alloc_thread_sem
  thread->p_th_cap = _t.cap();

  err = __alloc_thread_sem(thread, th_sem.get());
  if (err < 0)
    return -err;

  thread->p_thsem_cap = th_sem.cap();

  L4::Thread::Attr attr;
  l4_utcb_t *nt_utcb = (l4_utcb_t*)thread->p_tid;

  attr.bind(nt_utcb, L4Re::This_task);
  attr.pager(e->rm());
  attr.exc_handler(e->rm());
  if ((err = l4_error(_t->control(attr))) < 0)
    fprintf(stderr, "ERROR: l4 thread control returned: %d\n", err);

  l4_utcb_tcr_u(nt_utcb)->user[0] = l4_addr_t(thread);


  l4_umword_t *&_tos = (l4_umword_t*&)*tos;

  *(--_tos) = l4_addr_t(thread);
  *(--_tos) = 0; /* ret addr */
  *(--_tos) = l4_addr_t(f);


  _t->ex_regs(l4_addr_t(__pthread_new_thread_entry), l4_addr_t(_tos), 0);

  if (thread->p_start_args.start_routine
      && !(create_flags & PTHREAD_L4_ATTR_NO_START))
    {
      l4_sched_param_t sp = l4_sched_param(prio >= 0 ? prio : 2);
      sp.affinity = affinity;
      e->scheduler()->run_thread(_t.get(), sp);
    }

  // release the automatic capabilities
  _t.release();
  th_sem.release();
  return 0;
}

static int l4pthr_get_more_utcb()
{
  using namespace L4Re;

  l4_addr_t kumem = 0;
  Env const *e = Env::env();

  if (e->rm()->reserve_area(&kumem, L4_PAGESIZE,
                            Rm::Reserved | Rm::Search_addr))
    return 1;

  if (l4_error(e->task()->add_ku_mem(l4_fpage(kumem, L4_PAGESHIFT,
                                              L4_FPAGE_RW))))
    {
      e->rm()->free_area(kumem);
      return 1;
    }

  __l4_add_utcbs(kumem, kumem + L4_PAGESIZE);
  return 0;
}


static inline l4_utcb_t *mgr_alloc_utcb()
{
  l4_utcb_t *new_utcb = __pthread_first_free_handle;
  if (!new_utcb)
    return 0;

  __pthread_first_free_handle = (l4_utcb_t*)l4_utcb_tcr_u(new_utcb)->user[0];
  return new_utcb;
}

static inline void mgr_free_utcb(l4_utcb_t *u)
{
  if (!u)
    return;

  l4_utcb_tcr_u(u)->user[0] = l4_addr_t(__pthread_first_free_handle);
  __pthread_first_free_handle = u;
}

int __pthread_start_manager(pthread_descr mgr)
{
  int err;

  mgr->p_tid = mgr_alloc_utcb();

  err = __pthread_mgr_create_thread(mgr, &__pthread_manager_thread_tos,
                                    __pthread_manager, -1, 0, l4_sched_cpu_set(0, ~0, 1));
  if (err < 0)
    {
      fprintf(stderr, "ERROR: could not start pthread manager thread\n");
      exit(100);
    }

  __pthread_manager_request = mgr->p_th_cap;
  return 0;
}


static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
				 void * (*start_routine)(void *), void *arg)
{
  int err;
  pthread_descr new_thread;
  char *stack_addr;
  char * new_thread_bottom;
  pthread_t new_thread_id;
  char *guardaddr = NULL;
  size_t guardsize = 0, stksize = 0;
  int pagesize = L4_PAGESIZE;
  int saved_errno = 0;

#ifdef USE_TLS
  new_thread = (pthread*)_dl_allocate_tls (NULL);
  if (new_thread == NULL)
    return EAGAIN;
# if defined(TLS_DTV_AT_TP)
  /* pthread_descr is below TP.  */
  new_thread = (pthread_descr) ((char *) new_thread - TLS_PRE_TCB_SIZE);
# endif
#else
  /* Prevent warnings.  */
  new_thread = NULL;
#endif
#ifdef __NOT_FOR_L4__
  /* First check whether we have to change the policy and if yes, whether
     we can  do this.  Normally this should be done by examining the
     return value of the __sched_setscheduler call in pthread_start_thread
     but this is hard to implement.  FIXME  */
  if (attr != NULL && attr->__schedpolicy != SCHED_OTHER && geteuid () != 0)
    return EPERM;
#endif
  /* Find a free segment for the thread, and allocate a stack if needed */

  if (__pthread_first_free_handle == 0 && l4pthr_get_more_utcb())
    {
#ifdef USE_TLS
# if defined(TLS_DTV_AT_TP)
	  new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
# endif
	  _dl_deallocate_tls (new_thread, true);
#endif

      return EAGAIN;
    }

  l4_utcb_t *new_utcb = mgr_alloc_utcb();
  if (!new_utcb)
    return EAGAIN;

  new_thread_id = new_utcb;

  if (pthread_allocate_stack(attr, thread_segment(sseg),
                             pagesize, &stack_addr, &new_thread_bottom,
                             &guardaddr, &guardsize, &stksize) == 0)
    {
#ifdef USE_TLS
      new_thread->p_stackaddr = stack_addr;
#else
      new_thread = (pthread_descr) stack_addr;
#endif
    }
  else
    {
      mgr_free_utcb(new_utcb);
      return EAGAIN;
    }

  /* Allocate new thread identifier */
  /* Initialize the thread descriptor.  Elements which have to be
     initialized to zero already have this value.  */
#if !defined USE_TLS || !TLS_DTV_AT_TP
  new_thread->header.tcb = new_thread;
  new_thread->header.self = new_thread;
#endif
  new_thread->header.multiple_threads = 1;
  new_thread->p_tid = new_thread_id;
  new_thread->p_lock = handle_to_lock(new_utcb);
  new_thread->p_cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->p_canceltype = PTHREAD_CANCEL_DEFERRED;
#if !(USE_TLS && HAVE___THREAD)
  new_thread->p_errnop = &new_thread->p_errno;
  new_thread->p_h_errnop = &new_thread->p_h_errno;
#endif
  new_thread->p_guardaddr = guardaddr;
  new_thread->p_guardsize = guardsize;
  new_thread->p_inheritsched = attr ? attr->__inheritsched : 0;
  new_thread->p_alloca_cutoff = stksize / 4 > __MAX_ALLOCA_CUTOFF
				 ? __MAX_ALLOCA_CUTOFF : stksize / 4;
  /* Initialize the thread handle */
  __pthread_init_lock(handle_to_lock(new_utcb));
  /* Determine scheduling parameters for the thread */
  new_thread->p_sched_policy = -1;
  if (attr != NULL)
    {
      new_thread->p_detached = attr->__detachstate;
      new_thread->p_userstack = attr->__stackaddr_set;

      switch(attr->__inheritsched)
	{
	case PTHREAD_EXPLICIT_SCHED:
	  new_thread->p_sched_policy = attr->__schedpolicy;
	  new_thread->p_priority = attr->__schedparam.sched_priority;
	  break;
	case PTHREAD_INHERIT_SCHED:
	  break;
	}
    }
  int prio = -1;
  /* Set the scheduling policy and priority for the new thread, if needed */
  if (new_thread->p_sched_policy >= 0)
    {
      /* Explicit scheduling attributes were provided: apply them */
      prio = __pthread_l4_getprio(new_thread->p_sched_policy,
                                  new_thread->p_priority);
      /* Raise priority of thread manager if needed */
      __pthread_manager_adjust_prio(prio);
    }
  else if (manager_thread->p_sched_policy > 3)
    {
      /* Default scheduling required, but thread manager runs in realtime
         scheduling: switch new thread to SCHED_OTHER policy */
      prio = __pthread_l4_getprio(SCHED_OTHER, 0);
    }
  /* Finish setting up arguments to pthread_start_thread */
  new_thread->p_start_args.start_routine = start_routine;
  new_thread->p_start_args.arg = arg;
  /* Make the new thread ID available already now.  If any of the later
     functions fail we return an error value and the caller must not use
     the stored thread ID.  */
  *thread = new_thread_id;
  /* Do the cloning.  We have to use two different functions depending
     on whether we are debugging or not.  */
  err =  __pthread_mgr_create_thread(new_thread, &stack_addr,
                                     pthread_start_thread, prio,
                                     attr ? attr->create_flags : 0,
                                     attr ? attr->affinity : l4_sched_cpu_set(0, ~0, 1));
  saved_errno = err;

  /* Check if cloning succeeded */
  if (err < 0) {
    /* Free the stack if we allocated it */
    if (attr == NULL || !attr->__stackaddr_set)
      {
#ifdef NEED_SEPARATE_REGISTER_STACK
	size_t stacksize = ((char *)(new_thread->p_guardaddr)
			    - new_thread_bottom);
	munmap((caddr_t)new_thread_bottom,
	       2 * stacksize + new_thread->p_guardsize);
#elif _STACK_GROWS_UP
# ifdef USE_TLS
	size_t stacksize = guardaddr - stack_addr;
	munmap(stack_addr, stacksize + guardsize);
# else
	
	size_t stacksize = guardaddr - (char *)new_thread;
	munmap(new_thread, stacksize + guardsize);
# endif
#else
#ifdef USE_L4RE_FOR_STACK
        if (pthread_l4_free_stack(new_thread_bottom, guardaddr))
          fprintf(stderr, "ERROR: failed to free stack\n");
#else
# ifdef USE_TLS
	size_t stacksize = stack_addr - new_thread_bottom;
# else
	size_t stacksize = (char *)(new_thread+1) - new_thread_bottom;
# endif
	munmap(new_thread_bottom - guardsize, guardsize + stacksize);
#endif
#endif
      }
#ifdef USE_TLS
# if defined(TLS_DTV_AT_TP)
    new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
# endif
    _dl_deallocate_tls (new_thread, true);
#endif
    mgr_free_utcb(new_utcb);
    return saved_errno;
  }
  /* Insert new thread in doubly linked list of active threads */
  new_thread->p_prevlive = __pthread_main_thread;
  new_thread->p_nextlive = __pthread_main_thread->p_nextlive;
  __pthread_main_thread->p_nextlive->p_prevlive = new_thread;
  __pthread_main_thread->p_nextlive = new_thread;
  /* Set pid field of the new thread, in case we get there before the
     child starts. */
  return 0;
}


/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_free(pthread_descr th)
{
  pthread_handle handle;
  pthread_readlock_info *iter, *next;

  ASSERT(th->p_exited);
  /* Make the handle invalid */
  handle =  thread_handle(th->p_tid);
  __pthread_lock(handle_to_lock(handle), NULL);
  mgr_free_utcb(handle);
  __pthread_unlock(handle_to_lock(handle));

    {
      // free the semaphore and the thread
      L4Re::Util::Auto_cap<void>::Cap s = L4::Cap<void>(th->p_thsem_cap);
      L4Re::Util::Auto_cap<void>::Cap t = L4::Cap<void>(th->p_th_cap);
    }

  /* One fewer threads in __pthread_handles */

  /* Destroy read lock list, and list of free read lock structures.
     If the former is not empty, it means the thread exited while
     holding read locks! */

  for (iter = th->p_readlock_list; iter != NULL; iter = next)
    {
      next = iter->pr_next;
      free(iter);
    }

  for (iter = th->p_readlock_free; iter != NULL; iter = next)
    {
      next = iter->pr_next;
      free(iter);
    }

  /* If initial thread, nothing to free */
  if (!th->p_userstack)
    {
      size_t guardsize = th->p_guardsize;
      /* Free the stack and thread descriptor area */
      char *guardaddr = (char*)th->p_guardaddr;
#ifdef _STACK_GROWS_UP
# ifdef USE_TLS
      size_t stacksize = guardaddr - th->p_stackaddr;
# else
      size_t stacksize = guardaddr - (char *)th;
# endif
      guardaddr = (char *)th;
#else
      /* Guardaddr is always set, even if guardsize is 0.  This allows
	 us to compute everything else.  */
# ifdef USE_TLS
      //l4/size_t stacksize = th->p_stackaddr - guardaddr - guardsize;
# else
      //l4/size_t stacksize = (char *)(th+1) - guardaddr - guardsize;
# endif
# ifdef NEED_SEPARATE_REGISTER_STACK
      /* Take account of the register stack, which is below guardaddr.  */
      guardaddr -= stacksize;
      stacksize *= 2;
# endif
#endif
#ifdef USE_L4RE_FOR_STACK
      pthread_l4_free_stack(guardaddr + guardsize, guardaddr);
#else
      munmap(guardaddr, stacksize + guardsize);
#endif

    }

#ifdef USE_TLS
# if defined(TLS_DTV_AT_TP)
  th = (pthread_descr) ((char *) th + TLS_PRE_TCB_SIZE);
# endif
  _dl_deallocate_tls (th, true);
#endif
}

/* Handle threads that have exited */

static int pthread_exited(pthread_descr th)
{
  if (th->p_exited)
    return 0;

  int detached;
  /* Remove thread from list of active threads */
  th->p_nextlive->p_prevlive = th->p_prevlive;
  th->p_prevlive->p_nextlive = th->p_nextlive;
  /* Mark thread as exited, and if detached, free its resources */
  __pthread_lock(th->p_lock, NULL);
  th->p_exited = 1;
  /* If we have to signal this event do it now.  */
  detached = th->p_detached;
  __pthread_unlock(th->p_lock);
  if (detached)
    pthread_free(th);
  /* If all threads have exited and the main thread is pending on a
     pthread_exit, wake up the main thread and terminate ourselves. */
  if (main_thread_exiting &&
      __pthread_main_thread->p_nextlive == __pthread_main_thread) {
    restart(__pthread_main_thread);
    /* Same logic as REQ_MAIN_THREAD_EXIT. */
  }

  return detached;
}


/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_handle_free(pthread_t th_id)
{
  pthread_handle handle = thread_handle(th_id);
  pthread_descr th;

  __pthread_lock(handle_to_lock(handle), NULL);
  if (nonexisting_handle(handle, th_id)) {
    /* pthread_reap_children has deallocated the thread already,
       nothing needs to be done */
    __pthread_unlock(handle_to_lock(handle));
    return;
  }
  th = handle_to_descr(handle);
  __pthread_unlock(handle_to_lock(handle));
  if (!pthread_exited(th))
    pthread_free(th);
}

/* Send a signal to all running threads */

#if 0
static void pthread_kill_all_threads(int main_thread_also)
{
  UNIMPL("pthread_kill_all_threads");
#if 0
  pthread_descr th;
  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    kill(th->p_pid, sig);
  }
  if (main_thread_also) {
    kill(__pthread_main_thread->p_pid, sig);
  }
#endif
}
#endif

static void pthread_for_each_thread(void *arg,
    void (*fn)(void *, pthread_descr))
{
  pthread_descr th;

  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    fn(arg, th);
  }

  fn(arg, __pthread_main_thread);
}

/* Process-wide exit() */

static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
{
  //l4/pthread_descr th;
  __pthread_exit_requested = 1;
  __pthread_exit_code = exitcode;
#if 0
  /* A forced asynchronous cancellation follows.  Make sure we won't
     get stuck later in the main thread with a system lock being held
     by one of the cancelled threads.  Ideally one would use the same
     code as in pthread_atfork(), but we can't distinguish system and
     user handlers there.  */
  __flockfilelist();
  /* Send the CANCEL signal to all running threads, including the main
     thread, but excluding the thread from which the exit request originated
     (that thread must complete the exit, e.g. calling atexit functions
     and flushing stdio buffers). */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    kill(th->p_pid, __pthread_sig_cancel);
  }
  /* Now, wait for all these threads, so that they don't become zombies
     and their times are properly added to the thread manager's times. */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    waitpid(th->p_pid, NULL, __WCLONE);
  }
  __fresetlockfiles();
#endif
  restart(issuing_thread);
#ifdef THIS_IS_THE_ORIGINAL
  _exit(0);
#else
  // we do not do the exit path with kill and waitpid, so give the code here
  _exit(exitcode);
#endif
}

#if 0
/* Handler for __pthread_sig_cancel in thread manager thread */

void __pthread_manager_sighandler(int sig)
{
  int kick_manager = terminated_children == 0 && main_thread_exiting;
  terminated_children = 1;

  /* If the main thread is terminating, kick the thread manager loop
     each time some threads terminate. This eliminates a two second
     shutdown delay caused by the thread manager sleeping in the
     call to __poll(). Instead, the thread manager is kicked into
     action, reaps the outstanding threads and resumes the main thread
     so that it can complete the shutdown. */

  if (kick_manager) {
    struct pthread_request request;
    request.req_thread = 0;
    request.req_kind = REQ_KICK;
    TEMP_FAILURE_RETRY(write_not_cancel(__pthread_manager_request,
					(char *) &request, sizeof(request)));
  }
}
#endif
/* Adjust priority of thread manager so that it always run at a priority
   higher than all threads */

void __pthread_manager_adjust_prio(int thread_prio)
{
  if (!manager_thread)
    return;

  if (thread_prio <= manager_thread->p_priority)
    return;

  l4_sched_param_t sp = l4_sched_param(thread_prio, 0);
  L4Re::Env::env()->scheduler()->run_thread(L4::Cap<L4::Thread>(manager_thread->p_th_cap), sp);
  manager_thread->p_priority = thread_prio;
}
