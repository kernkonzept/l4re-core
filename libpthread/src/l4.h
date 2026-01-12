#pragma once

#include <l4/sys/semaphore>

typedef L4::Semaphore Th_sem_cap;

inline int __alloc_thread_sem(pthread_descr th, L4::Cap<Th_sem_cap> const &c)
{
  return l4_error(L4Re::Env::env()->factory()->create(c));
}
