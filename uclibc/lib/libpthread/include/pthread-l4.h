#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <pthread.h>

enum
{
  PTHREAD_L4_ATTR_NO_START = 0x0001,
};

__BEGIN_DECLS

l4_cap_idx_t pthread_l4_cap(pthread_t t);

void pthread_l4_for_each_thread(void (*fn)(pthread_t));

static inline l4_utcb_t *pthread_l4_utcb(pthread_t t);

int pthread_l4_start(pthread_t thread, void *(*func)(void *), void *arg);

/* Old-named version of pthread_l4_cap(), obsolete. */
static inline l4_cap_idx_t pthread_getl4cap(pthread_t t)
   L4_DEPRECATED("pthread_getl4cap() has been renamed to pthread_l4_cap()");

__END_DECLS

#ifdef __cplusplus
#include <l4/sys/thread>

namespace Pthread { namespace L4 {

inline ::L4::Cap< ::L4::Thread> cap(pthread_t t)
{ return ::L4::Cap< ::L4::Thread>(pthread_l4_cap(t)); }

inline l4_utcb_t *utcb(pthread_t t)
{ return pthread_l4_utcb(t); }

inline int start(pthread_t thread, void *(*start_routine)(void *), void *arg)
{ return pthread_l4_start(thread, start_routine, arg); }


}} // namespace L4, namespace Pthread

#endif

/* Implementations */

static inline
l4_utcb_t *
pthread_l4_utcb(pthread_t t)
{ return (l4_utcb_t *) t; }

/* Deprecated version of pthread_l4_cap() */
static inline l4_cap_idx_t pthread_getl4cap(pthread_t t)
{ return pthread_l4_cap(t); }
