#ifndef _PTHREAD_IMPL_H
#define _PTHREAD_IMPL_H

// NOTE: Need all these includes for compatibility, some other source files that
// include pthread_impl.h implicitly use the includes from here...
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include "libc.h"
#include "syscall.h"
#include "atomic.h"

#include "pthread-api.h"

#ifndef TP_OFFSET
#define TP_OFFSET 0
#endif

#ifndef DTP_OFFSET
#define DTP_OFFSET 0
#endif

#ifndef tls_mod_off_t
#define tls_mod_off_t size_t
#endif

hidden void __set_tp(void *tls_tp);

void *__tls_get_addr(tls_mod_off_t *);
hidden int __init_tp(pthread_descr);
hidden pthread_descr __copy_tls(unsigned char *);
hidden void __reset_tls();

hidden void __acquire_ptc(void);
hidden void __release_ptc(void);
hidden void __inhibit_ptc(void);

extern hidden unsigned __default_stacksize;
extern hidden unsigned __default_guardsize;

#define DEFAULT_STACK_SIZE 131072
#define DEFAULT_GUARD_SIZE 8192

#define DEFAULT_STACK_MAX (8<<20)
#define DEFAULT_GUARD_MAX (1<<20)

#endif
