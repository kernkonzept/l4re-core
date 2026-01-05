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
#include <l4/re/itas>
#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/rm>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/sys/capability>
#include <l4/sys/debugger.h>
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

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

extern "C" void __pthread_new_thread_entry(void);

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

/* Flag set when the initial thread is blocked on pthread_exit waiting
   for all other threads to terminate */

static int main_thread_exiting;

/* Counter used to generate unique thread identifier.
   Thread identifier is pthread_threads_counter + segment. */

//l4/static pthread_t pthread_threads_counter;

/* Forward declarations */

static int pthread_handle_create(pthread_descr creator, const pthread_attr_t *attr,
                                 void * (*start_routine)(void *), void *arg);
static void pthread_handle_free(pthread_t th_id);
static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode);
//l4/static void pthread_kill_all_threads(int main_thread_also);

static int pthread_handle_thread_exit(pthread_descr th);

/* The server thread managing requests for thread creation and termination */

static pthread_descr pthread_first_thread(void)
{
  return __pthread_main_thread;
}

static pthread_descr pthread_next_thread(pthread_descr th)
{
  pthread_descr next_th = th->p_nextlive;
  return next_th != __pthread_main_thread ? next_th : nullptr;
}

static l4_pthread_mgr_iface_t _pthread_mgr_iface = {
  .first_thread = pthread_first_thread,
  .next_thread = pthread_next_thread,
};

int
__attribute__ ((noreturn))
__pthread_manager(void *arg)
{
  pthread_descr self = manager_thread = (pthread_descr)arg;
  struct pthread_request request;

  __l4_utcb_mark_used(l4_utcb());

#if defined(TLS_TCB_AT_TP)
  TLS_INIT_TP(self, 0);
#elif defined(TLS_DTV_AT_TP)
  TLS_INIT_TP((char *)self + TLS_PRE_TCB_SIZE, 0);
#else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
  /* If we have special thread_self processing, initialize it.  */
  INIT_THREAD_SELF(self, 1);
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
	    pthread_handle_create(request.req_thread,
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
	  do_reply = 1;
	  break;
	case REQ_KICK:
	  /* This is just a prod to get the manager to reap some
	     threads right away, avoiding a potential delay at shutdown. */
	  break;
	case REQ_EXEC_IN_MANAGER:
          request.req_args.exec_in_mgr.fn(&_pthread_mgr_iface,
                                          request.req_args.exec_in_mgr.arg);
          restart(request.req_thread);
	  do_reply = 1;
	  break;
        case REQ_THREAD_EXIT:
            {
              if (!pthread_handle_thread_exit(request.req_thread))
                {
                  auto th = request.req_thread;
                  /* Thread still waiting to be joined. Only release
                     L4 resources for now. */
                  // Keep the cap slot allocated and let pthread_free() do the
                  // final cleanup. This way, we can safely check the
                  // thread cap index for kernel object presence until
                  // pthread_join/detach() was called.
                  l4_fpage_t del_obj[2] =
                    {
                      L4::Cap<void>(th->p_thsem_cap).fpage(),
                      L4::Cap<void>(th->p_th_cap).fpage()
                    };
                  L4Re::Env::env()->task()->unmap_batch(del_obj, 2,
                                                        L4_FP_DELETE_OBJ);
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
  INIT_THREAD_SELF(self, 1);

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
  __l4_utcb_mark_used(l4_utcb());

  pthread_descr self = (pthread_descr) arg;
#if defined(TLS_TCB_AT_TP)
  TLS_INIT_TP(self, 0);
#elif defined(TLS_DTV_AT_TP)
  TLS_INIT_TP((char *)self + TLS_PRE_TCB_SIZE, 0);
#else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif

  void * outcome;
  /* Initialize special thread_self processing, if any.  */
  INIT_THREAD_SELF(self, self->p_nr);

  /* Run the thread code */
  outcome = self->p_start_args.start_routine(THREAD_GETMEM(self,
							   p_start_args.arg));
  /* Exit with the given return value */
  __pthread_do_exit(outcome, (char *)CURRENT_STACK_FRAME);
}

static int pthread_l4_free_stack(void *stack_addr, void *guardaddr)
{
  L4Re::Env const *e = L4Re::Env::env();
  int err;
  L4::Cap<L4Re::Dataspace> ds;

  err = e->rm()->detach(stack_addr, &ds);
  if (err < 0)
    return err;

  if (err == L4Re::Rm::Detached_ds)
    L4Re::Util::cap_alloc.free(ds, L4Re::This_task);

  return e->rm()->free_area((l4_addr_t)guardaddr);
}

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

  /* TLS cannot work with fixed thread descriptor addresses.  */
  assert (default_new_thread == NULL);

  if (attr != NULL && attr->__stackaddr_set)
    {
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
      new_thread = (pthread_descr) attr->__stackaddr;
      new_thread_bottom = (char *) attr->__stackaddr - attr->__stacksize;
      guardaddr = new_thread_bottom;
      guardsize = 0;
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

      map_addr = 0;
      L4Re::Env const *e = L4Re::Env::env();
      long err;

#ifdef CONFIG_MMU
      if (e->rm()->reserve_area(&map_addr, stacksize + guardsize,
	                        L4Re::Rm::F::Search_addr) < 0)
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
      err = e->rm()->attach(&new_thread_bottom, stacksize,
                            L4Re::Rm::F::In_area | L4Re::Rm::F::RW,
                            L4::Ipc::make_cap_rw(ds), 0);

      if (err < 0)
	{
	  L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
	  e->rm()->free_area(l4_addr_t(map_addr));
	  return -1;
	}
#else
      L4::Cap<L4Re::Dataspace> ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
      if (!ds.is_valid())
        return -1;

      err = e->mem_alloc()->alloc(stacksize, ds);
      if (err < 0)
        {
          L4Re::Util::cap_alloc.free(ds);
          return -1;
        }

      err = e->rm()->attach(&map_addr, stacksize,
                            L4Re::Rm::F::Search_addr | L4Re::Rm::F::RW,
                            L4::Ipc::make_cap_rw(ds), 0);
      if (err < 0)
        {
          L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
          return -1;
        }

      guardaddr = static_cast<char *>(map_addr) - guardsize;
      new_thread_bottom = static_cast<char *>(map_addr);
#endif

      new_thread = ((pthread_descr) (new_thread_bottom + stacksize));
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
  auto _t = L4Re::Util::make_unique_cap<L4::Thread>();
  if (!_t.is_valid())
    return -ENOMEM;

  auto th_sem = L4Re::Util::make_unique_cap<Th_sem_cap>();
  if (!th_sem.is_valid())
    return -ENOMEM;

  int err = l4_error(e->factory()->create(_t.get()));
  if (err < 0)
    return err;

  // needed by __alloc_thread_sem
  thread->p_th_cap = _t.cap();

  err = __alloc_thread_sem(thread, th_sem.get());
  if (err < 0)
    return err;

  thread->p_thsem_cap = th_sem.cap();

  l4_utcb_t *nt_utcb = (l4_utcb_t*)thread->p_tid;

  auto itas = L4Re::Env::env()->itas();
  pthread_descr self = thread_self();
  bool registered = itas && itas
      ->register_thread(L4::Cap<L4::Thread>(self->p_th_cap),
                        _t.get(),
                        reinterpret_cast<l4_addr_t>(nt_utcb)) >= 0;
  if (!registered)
    {
      // Without ITAS services, we can still bind the thread ourselves...
      L4::Thread::Attr attr;
      attr.bind(nt_utcb, L4Re::This_task);
      attr.pager(e->rm());
      attr.exc_handler(e->rm());

      if ((err = l4_error(_t->control(attr))) < 0)
        {
          fprintf(stderr, "ERROR: thread control returned: %d\n", err);
          return err;
        }
    }

  l4_utcb_tcr_u(nt_utcb)->user[0] = l4_addr_t(thread);

  l4_umword_t *&_tos = (l4_umword_t*&)*tos;

  *(--_tos) = l4_addr_t(thread);
  *(--_tos) = 0; /* ret addr */
  *(--_tos) = l4_addr_t(f);

  l4_umword_t flags = 0;
#if defined(__arm__) || defined(__aarch64__)
  {
    // Inherit exception level on Arm. By default all threads run in EL0 but if
    // the executable chose to use EL1 we must inherit the EL!
    l4_umword_t ip = ~0UL;
    l4_umword_t sp = ~0UL;
    err = l4_error(L4::Cap<L4::Thread>()->ex_regs(&ip, &sp, &flags));
    if (err < 0)
      {
        fprintf(stderr, "ERROR: exregs returned error: %d\n", err);
        if (registered)
          itas->unregister_thread(_t.get());
        return err;
      }
  }
#endif

  err = l4_error(_t->ex_regs(l4_addr_t(__pthread_new_thread_entry),
                             l4_addr_t(_tos), flags));

  if (err < 0)
    {
      fprintf(stderr, "ERROR: exregs returned error: %d\n", err);
      if (registered)
        itas->unregister_thread(_t.get());
      return err;
    }

  if (thread->p_start_args.start_routine
      && !(create_flags & PTHREAD_L4_ATTR_NO_START))
    {
      l4_sched_param_t sp = l4_sched_param(prio >= 0 ? prio : 2);
      sp.affinity = affinity;
      err = l4_error(e->scheduler()->run_thread(_t.get(), sp));
      if (err < 0)
        {
          fprintf(stderr,
                  "ERROR: could not start thread, run_thread returned %d\n",
                  err);
          if (registered)
            itas->unregister_thread(_t.get());
          return err;
        }
    }

  // release the automatic capabilities
  _t.release();
  th_sem.release();
  return 0;
}

/*
 * Add more free UTCBs by allocating more KU memory. Return the first UTCB of
 * the list to the caller. Return nullptr if the allocation failed.
 */
static l4_utcb_t *l4pthr_allocate_more_utcbs_and_claim_utcb()
{
  using namespace L4Re;

  l4_addr_t kumem = 0;
  Env const *e = Env::env();

#ifdef CONFIG_MMU
  // On MMU systems, user space chooses the spot in the virtual address space.
  if (e->rm()->reserve_area(&kumem, L4_PAGESIZE,
                            Rm::F::Reserved | Rm::F::Search_addr))
    return nullptr;

  l4_fpage_t fp = l4_fpage(kumem, L4_PAGESHIFT, L4_FPAGE_RW);
  if (l4_error(e->task()->add_ku_mem(&fp)))
    {
      e->rm()->free_area(kumem);
      return nullptr;
    }
#else
  // On systems without MMU the kernel determines the actual location.
  l4_fpage_t fp = l4_fpage(0, L4_PAGESHIFT, L4_FPAGE_RW);
  if (l4_error(e->task()->add_ku_mem(&fp)))
    return nullptr;
  kumem = l4_fpage_memaddr(fp);

  // The kernel allocated the address so it is known to be valid. The
  // reservation should never fail, unless something is really broken.
  long err = e->rm()->reserve_area(&kumem, L4_PAGESIZE, Rm::F::Reserved);
  if (err < 0)
    fprintf(stderr,
            "ERROR: could not reserve ku_mem area, reserve_area returned %ld\n",
            err);
#endif

  __l4_add_utcbs(kumem, kumem + L4_PAGESIZE);

  l4_utcb_t *u = __pthread_first_free_utcb;
  __pthread_first_free_utcb = __l4_utcb_get_next_free(u);
  return u;
}


/*
 * Return the pointer to the first unused UTCB in the UTCB free list. If no
 * UTCB is currently unused, return nullptr.
 */
static inline l4_utcb_t *claim_unused_utcb()
{
  l4_utcb_t *prev_u = nullptr;

  for (l4_utcb_t *u = __pthread_first_free_utcb; u; u = __l4_utcb_get_next_free(u))
    {
      if (__l4_utcb_is_usable_now(u))
        {
          if (prev_u)
            __l4_utcb_set_next_free(prev_u, __l4_utcb_get_next_free(u));
          else
            __pthread_first_free_utcb = __l4_utcb_get_next_free(u);
          return u;
        }

      prev_u = u;
    }

  return nullptr;
}

/*
 * Enqueue the UTCB as first element of the UTCB free list.
 */
static inline void mgr_free_utcb(l4_utcb_t *u)
{
  if (!u)
    return;

  l4_thread_regs_t *tcr = l4_utcb_tcr_u(u);
  tcr->user[0] = 0;
  __l4_utcb_set_next_free(u, __pthread_first_free_utcb);
  __pthread_first_free_utcb = u;
}

int __pthread_start_manager(pthread_descr mgr)
{
  int err;

  // This succeeds because of adding UTCBs in __pthread_initialize_minimal().
  mgr->p_tid = thread_id(claim_unused_utcb());

  err = __pthread_mgr_create_thread(mgr, &__pthread_manager_thread_tos,
                                    __pthread_manager, -1, 0, l4_sched_cpu_set(0, ~0, 1));
  if (err < 0)
    {
      fprintf(stderr, "ERROR: could not start pthread manager thread (err=%d)\n", err);
      exit(100);
    }

  __pthread_manager_request = mgr->p_th_cap;

  char s[15] = "pthread-mgr";
  if (__progname && __progname[0])
    {
      char *t = strstr(__progname, "rom/");
      s[0] = '~';
      strncpy(s + 1, t ? t + 4 : __progname, sizeof(s) - 2);
      s[sizeof(s) - 1] = 0;
    }

  l4_debugger_set_object_name(__pthread_manager_request, s);

  return 0;
}


static int pthread_handle_create(pthread_descr creator, const pthread_attr_t *attr,
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

  new_thread = (pthread*)_dl_allocate_tls (NULL);
  if (new_thread == NULL)
    return EAGAIN;
#if defined(TLS_DTV_AT_TP)
  /* pthread_descr is below TP.  */
  new_thread = (pthread_descr) ((char *) new_thread - TLS_PRE_TCB_SIZE);
#endif
  /* Find a free segment for the thread, and allocate a stack if needed */

  l4_utcb_t *new_utcb = claim_unused_utcb();
  if (!new_utcb)
    new_utcb = l4pthr_allocate_more_utcbs_and_claim_utcb();
  if (!new_utcb)
    {
# if defined(TLS_DTV_AT_TP)
	  new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
# endif
	  _dl_deallocate_tls (new_thread, true);
      return EAGAIN;
    }

  new_thread_id = thread_id(new_utcb);

  if (pthread_allocate_stack(attr, thread_segment(sseg),
                             pagesize, &stack_addr, &new_thread_bottom,
                             &guardaddr, &guardsize, &stksize) == 0)
    {
      new_thread->p_stackaddr = stack_addr;
    }
  else
    {
#if defined(TLS_DTV_AT_TP)
	  new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
#endif
	  _dl_deallocate_tls (new_thread, true);
      mgr_free_utcb(new_utcb);
      return EAGAIN;
    }

  /* Allocate new thread identifier */
  /* Initialize the thread descriptor.  Elements which have to be
     initialized to zero already have this value.  */
#if !TLS_DTV_AT_TP
  new_thread->header.tcb = new_thread;
  new_thread->header.self = new_thread;
#endif
  new_thread->p_tid = new_thread_id;
  new_thread->p_lock = handle_to_lock(new_utcb);
  new_thread->p_cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->p_canceltype = PTHREAD_CANCEL_DEFERRED;
  new_thread->p_guardaddr = guardaddr;
  new_thread->p_guardsize = guardsize;
  new_thread->p_inheritsched = attr ? attr->__inheritsched : PTHREAD_INHERIT_SCHED;
  /* Initialize the thread handle */
  __pthread_init_lock(handle_to_lock(new_utcb));
  /* Determine scheduling parameters for the thread */
  // If no attributes are provided, pthread_create uses default values as
  // described in pthread_attr_init. PTHREAD_INHERIT_SCHED is the default.

  new_thread->p_sched_policy = creator->p_sched_policy;
  new_thread->p_priority = creator->p_priority;

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
  creator->p_retval = reinterpret_cast<void *>(new_thread_id);
  /* Do the cloning.  We have to use two different functions depending
     on whether we are debugging or not.  */
  err =  __pthread_mgr_create_thread(new_thread, &stack_addr,
                                     pthread_start_thread, prio,
                                     attr ? attr->create_flags : 0,
                                     attr ? attr->affinity : l4_sched_cpu_set(0, ~0, 1));
  saved_errno = -err;

  /* Check if cloning succeeded */
  if (err < 0) {
    /* Free the stack if we allocated it */
    if (attr == NULL || !attr->__stackaddr_set)
      {
        if (pthread_l4_free_stack(new_thread_bottom, guardaddr))
          fprintf(stderr, "ERROR: failed to free stack\n");
      }
#if defined(TLS_DTV_AT_TP)
    new_thread = (pthread_descr) ((char *) new_thread + TLS_PRE_TCB_SIZE);
#endif
    _dl_deallocate_tls (new_thread, true);
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

  /* Make the handle invalid */
  handle =  thread_handle(th->p_tid);
  __pthread_lock(handle_to_lock(handle), NULL);
  assert(th->p_tid != 0);
  th->p_tid = 0;
  mgr_free_utcb(handle);
  __pthread_unlock(handle_to_lock(handle));

  auto itas = L4Re::Env::env()->itas();
  if (itas)
    itas->unregister_thread(L4::Cap<L4::Thread>(th->p_th_cap));

  {
    // free the semaphore and the thread
    L4Re::Util::Unique_del_cap<void> s(L4::Cap<void>(th->p_thsem_cap));
    L4Re::Util::Unique_del_cap<void> t(L4::Cap<void>(th->p_th_cap));
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
      /* Guardaddr is always set, even if guardsize is 0.  This allows
	 us to compute everything else.  */
      //l4/size_t stacksize = th->p_stackaddr - guardaddr - guardsize;
      pthread_l4_free_stack(guardaddr + guardsize, guardaddr);
    }

# if defined(TLS_DTV_AT_TP)
  th = (pthread_descr) ((char *) th + TLS_PRE_TCB_SIZE);
# endif
  _dl_deallocate_tls (th, true);
}

/*
 * Handle threads that have exited
 *
 * Return true if the thread has been freed due to being detached.
 */

static int pthread_handle_thread_exit(pthread_descr th)
{
  if (th->p_exited)
    return 0;

  assert(th->p_terminated);

  int detached;
  /* Remove thread from list of active threads */
  th->p_nextlive->p_prevlive = th->p_prevlive;
  th->p_prevlive->p_nextlive = th->p_nextlive;
  /* Mark thread as exited, and if detached, free its resources */
  __pthread_lock(th->p_lock, NULL);
  th->p_exited = 1;
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
  if (!pthread_handle_thread_exit(th))
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

/* Process-wide exit() */

static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
{
  pthread_descr th;
  __pthread_exit_requested = 1;
  __pthread_exit_code = exitcode;
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive)
    {
      __l4_kill_thread(th->p_th_cap);
    }

  // let caller continue
  if (l4_error(l4_ipc_send(L4_INVALID_CAP | L4_SYSF_REPLY,
                           l4_utcb(),
                           l4_msgtag(0, 0, 0, 0),
                           L4_IPC_SEND_TIMEOUT_0)))
    // assume caller has quit (and will not continue exit())
    _exit(0);
}

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
