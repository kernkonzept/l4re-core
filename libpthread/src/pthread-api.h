#pragma once

#include <l4/sys/compiler.h>

/* The type of thread descriptors */
typedef struct pthread *pthread_descr;

#include "pthread-api-libc.h"

L4_BEGIN_DECLS

L4_HIDDEN void pthread_onexit_process(int retcode, void *arg);

typedef struct l4_pthread_mgr_iface
{
  pthread_descr (*first_thread)(void);
  pthread_descr (*next_thread)(pthread_descr);
} l4_pthread_mgr_iface_t;

/**
 * If the manager has not yet been started, i.e. because no thread has been
 * created yet, this executes the function locally in context of the caller.
 */
void pthread_l4_exec_in_manager(void (*fn)(l4_pthread_mgr_iface_t const *mgr, void *arg), void *arg);


L4_END_DECLS
