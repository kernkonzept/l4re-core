#pragma once

#include <l4/sys/compiler.h>

typedef struct l4_pthread_mgr_iface
{
  pthread_descr (*first_thread)(void);
  pthread_descr (*next_thread)(pthread_descr);
} l4_pthread_mgr_iface_t;

L4_BEGIN_DECLS

/**
 * If the manager has not yet been started, i.e. because no thread has been
 * created yet, this executes the function locally in context of the caller.
 */
void pthread_l4_exec_in_manager(void (*fn)(l4_pthread_mgr_iface_t const *mgr, void *arg), void *arg);


L4_END_DECLS
