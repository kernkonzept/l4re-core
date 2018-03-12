#ifndef PT_EI
#define PT_EI inline
#endif

#include "internals.h"

#include <l4/sys/capability>
#include <l4/sys/thread>
#include <l4/re/env>
#include <l4/sys/factory>
#include <l4/re/util/cap_alloc>
#include <l4/sys/kdebug.h>
#include <l4/sys/scheduler>

#include <pthread-l4.h>
#include <errno.h>
#include "spinlock.h"

#include "l4.h"


l4_cap_idx_t pthread_l4_cap(pthread_t thread_id)
{
  __volatile__ pthread_descr self = thread_self();
  pthread_handle handle = thread_handle(thread_id);
  pthread_descr th;

  __pthread_lock(handle_to_lock(handle), self);
  if (nonexisting_handle(handle, thread_id)) {
    __pthread_unlock(handle_to_lock(handle));
    return L4_INVALID_CAP;
  }
  l4_cap_idx_t c;
  th = handle_to_descr(handle);
  c = th->p_th_cap;
  __pthread_unlock(handle_to_lock(handle));
  return c;
}

static void cb(void *arg, pthread_descr th)
{
  void (*fn)(pthread_t) = (void (*)(pthread_t))arg;

  fn(th->p_tid);
}

void pthread_l4_for_each_thread(void (*fn)(pthread_t))
{
  struct pthread_request request;

  request.req_thread = thread_self();
  request.req_kind = REQ_FOR_EACH_THREAD;
  request.req_args.for_each.arg = (void *)fn;
  request.req_args.for_each.fn = cb;

  __pthread_send_manager_rq(&request, 1);
}

int __pthread_l4_initialize_main_thread(pthread_descr th)
{
  L4Re::Env *env = const_cast<L4Re::Env*>(L4Re::Env::env());
  if (!env)
    return -L4_ENODEV;

  L4::Cap<Th_sem_cap> s(env->first_free_cap() << L4_CAP_SHIFT);
  if (!s.is_valid() || !s.cap())
    return -L4_ENOMEM;

  // needed by __alloc_thread_sem
  th->p_th_cap = env->main_thread().cap();

  int err = __alloc_thread_sem(th, s);
  if (err < 0)
    return err;

  env->first_free_cap((s.cap() + L4_CAP_OFFSET) >> L4_CAP_SHIFT);

  th->p_thsem_cap = s.cap();

  th->p_sched_policy = SCHED_L4;
  th->p_priority = 0x10;
  th->p_affinity_mask[0] = ~0ul;

  th->p_lock = handle_to_lock(l4_utcb());
  th->p_tid  = l4_utcb();
  l4_utcb_tcr()->user[0] = l4_addr_t(th);

  return 0;
}


int __attribute__((weak)) __pthread_sched_idle_prio    = 0x01;
int __attribute__((weak)) __pthread_sched_other_prio   = 0x02;
int __attribute__((weak)) __pthread_sched_rr_prio_min  = 0x40;
int __attribute__((weak)) __pthread_sched_rr_prio_max  = 0xf0;


int __pthread_setschedparam(pthread_t thread, int policy,
                            const struct sched_param *param) throw()
{
  pthread_handle handle = thread_handle(thread);
  pthread_descr th;

  __pthread_lock(handle_to_lock(handle), NULL);
  if (__builtin_expect (invalid_handle(handle, thread), 0)) {
    __pthread_unlock(handle_to_lock(handle));
    return ESRCH;
  }
  th = handle_to_descr(handle);
  int prio = __pthread_l4_getprio(policy, param->sched_priority);
  if (prio < 0)
    {
      __pthread_unlock(handle_to_lock(handle));
      return EINVAL;
    }

  th->p_sched_policy = policy;
  th->p_priority = param->sched_priority;

    {
      L4::Cap<L4::Thread> t(th->p_th_cap);
      l4_sched_param_t sp = l4_sched_param(prio, 0);
      L4Re::Env::env()->scheduler()->run_thread(t, sp);
    }
  __pthread_unlock(handle_to_lock(handle));

  if (__pthread_manager_request > 0)
    __pthread_manager_adjust_prio(prio);

  return 0;
}
strong_alias (__pthread_setschedparam, pthread_setschedparam)

int __pthread_getschedparam(pthread_t thread, int *policy,
                            struct sched_param *param) throw()
{
  pthread_handle handle = thread_handle(thread);
  int pol, prio;

  __pthread_lock(handle_to_lock(handle), NULL);
  if (__builtin_expect (invalid_handle(handle, thread), 0)) {
    __pthread_unlock(handle_to_lock(handle));
    return ESRCH;
  }

  pol = handle_to_descr(handle)->p_sched_policy;
  prio = handle_to_descr(handle)->p_priority;
  __pthread_unlock(handle_to_lock(handle));

  *policy = pol;
  param->sched_priority = prio;

  return 0;
}
strong_alias (__pthread_getschedparam, pthread_getschedparam)

int pthread_setaffinity_np(pthread_t __th, size_t __cpusetsize,
                           const cpu_set_t *__cpuset)
{
  pthread_handle handle = thread_handle(__th);

  if (__cpusetsize < sizeof(__cpuset->__bits[0]))
    return EINVAL;

  __pthread_lock(handle_to_lock(handle), NULL);

  pthread_descr th = handle_to_descr(handle);

  L4::Cap<L4::Thread> t(th->p_th_cap);
  l4_sched_param_t sp = l4_sched_param(th->p_priority, 0);
  static_assert(sizeof(__cpuset->__bits[0]) == sizeof(l4_umword_t),
                "Size mismatch");
  th->p_affinity_mask[0] = __cpuset->__bits[0];
  sp.affinity = l4_sched_cpu_set(0, 0, __cpuset->__bits[0]);
  int e = l4_error(L4Re::Env::env()->scheduler()->run_thread(t, sp));

  __pthread_unlock(handle_to_lock(handle));

  return -e;
}

int
pthread_getaffinity_np(pthread_t th, size_t cpusetsize, cpu_set_t *cpuset)
{
  pthread_handle handle = thread_handle(th);

  if (cpusetsize < sizeof(cpuset->__bits[0]))
    return EINVAL;

  __pthread_lock(handle_to_lock(handle), NULL);
  cpuset->__bits[0] = handle_to_descr(handle)->p_affinity_mask[0];
  __pthread_unlock(handle_to_lock(handle));

  return 0;
}

int pthread_l4_start(pthread_t th, void *(*func)(void *), void *arg)
{
  pthread_handle handle = thread_handle(th);
  __pthread_lock(handle_to_lock(handle), NULL);

  if (nonexisting_handle(handle, th))
    {
      __pthread_unlock(handle_to_lock(handle));
      return -EINVAL;
    }

  pthread_descr thread = handle_to_descr(handle);
  if (thread->p_start_args.start_routine)
    {
      __pthread_unlock(handle_to_lock(handle));
      return -EBUSY;
    }

  thread->p_start_args.start_routine = func;
  thread->p_start_args.arg = arg;

  int prio;
  if (thread->p_sched_policy < 0)
    /* Default scheduling required, but thread manager runs in realtime
       scheduling: switch new thread to SCHED_OTHER policy */
    prio = __pthread_l4_getprio(SCHED_OTHER, 0);
  else
    prio = __pthread_l4_getprio(thread->p_sched_policy,
                                thread->p_priority);

  l4_sched_param_t sp = l4_sched_param(prio >= 0 ? prio : 2);
  int res = l4_error(L4Re::Env::env()->scheduler()
    ->run_thread(L4::Cap<L4::Thread>(thread->p_th_cap), sp));

  __pthread_unlock(handle_to_lock(handle));
  return res;
}

