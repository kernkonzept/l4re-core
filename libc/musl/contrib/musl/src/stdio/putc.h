#include "stdio_impl.h"
#include "pthread_impl.h"

#ifdef NOT_FOR_L4

#ifdef __GNUC__
__attribute__((__noinline__))
#endif
static int locking_putc(int c, FILE *f)
{
	if (a_cas(&f->lock, 0, MAYBE_WAITERS-1)) __lockfile(f);
	c = putc_unlocked(c, f);
	if (a_swap(&f->lock, 0) & MAYBE_WAITERS)
		__wake(&f->lock, 1, 1);
	return c;
}

static inline int do_putc(int c, FILE *f)
{
	int l = f->lock;
	if (l < 0 || l && (l & ~MAYBE_WAITERS) == __pthread_self()->tid)
		return putc_unlocked(c, f);
	return locking_putc(c, f);
}

#elif defined(L4_MINIMAL_LIBC)

static inline int do_putc(int c, FILE *f)
{
	return putc_unlocked(c, f);
}

#else

static inline int do_putc(int c, FILE *f)
{
	if (f->needs_lock < 0)
		return putc_unlocked(c, f);

	// TODO: Optimize the case that we already hold the lock?
	pthread_mutex_lock(&f->lock);
	int r = putc_unlocked(c, f);
	pthread_mutex_unlock(&f->lock);
	return r;
}

#endif
