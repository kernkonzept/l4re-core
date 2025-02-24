#include <features.h>
#include <errno.h>
#ifdef __UCLIBC_HAS_THREADS__
#error These are no-threading stubs!
#endif

// These stubs are required by Clang libunwind / RWMutex.hpp unless
// _LIBUNWIND_HAS_NO_THREADS is defined there.
//
// We cannot include <pthread.h> with __UCLIBC_HAS_THREADS__ due to
// conflicts with libc-internal.h.

// These are not really structs but that doesn't matter here.
struct pthread_rwlock_t;

extern "C" int pthread_rwlock_rdlock(pthread_rwlock_t *) noexcept;
extern "C" int pthread_rwlock_wrlock(pthread_rwlock_t *) noexcept;
extern "C" int pthread_rwlock_unlock(pthread_rwlock_t *) noexcept;

int
pthread_rwlock_rdlock(pthread_rwlock_t *) noexcept
{
  return 0;
}

int
pthread_rwlock_wrlock(pthread_rwlock_t *) noexcept
{
  return 0;
}

int
pthread_rwlock_unlock(pthread_rwlock_t *) noexcept
{
  return 0;
}
