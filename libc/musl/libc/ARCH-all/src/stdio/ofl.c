#include "pthread_impl.h"
#include "stdio_impl.h"
#include "lock.h"

static FILE *ofl_head;
#ifndef L4_MINIMAL_LIBC
static libc_lock_t ofl_lock;
#endif

FILE **__ofl_lock()
{
	LOCK(ofl_lock);
	return &ofl_head;
}

void __ofl_unlock()
{
	UNLOCK(ofl_lock);
}
