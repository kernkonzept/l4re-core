#include "pthread_impl.h"
#include "lock.h"

// TODO: Do we really want to include libpthread internal headers here?
#include "internals.h"
#include "spinlock.h"

void __lock(libc_lock_t *l)
{
	int need_locks = libc.need_locks;
	if (!need_locks) return;

	__pthread_lock(l, __pthread_thread_self());
}

void __unlock(libc_lock_t *l)
{
	int need_locks = libc.need_locks;
	if (!need_locks) return;

	__pthread_unlock(l);
}
