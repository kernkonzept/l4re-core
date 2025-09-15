#ifndef LOCK_H
#define LOCK_H

#ifdef NOT_FOR_L4
hidden void __lock(volatile int *);
hidden void __unlock(volatile int *);
#define LOCK(x) __lock(x)
#define UNLOCK(x) __unlock(x)
#elif defined(L4_MINIMAL_LIBC)
typedef int libc_lock_t;
#define LOCK(x)
#define UNLOCK(x)
#else
// TODO: Maybe instead define a layout compatible struct, which we then cast in __lock/__unlock?
#include <pthread.h>

typedef struct _pthread_fastlock libc_lock_t;
hidden void __lock(libc_lock_t *);
hidden void __unlock(libc_lock_t *);
#define LOCK(x) __lock(&x)
#define UNLOCK(x) __unlock(&x)
#endif

#endif
