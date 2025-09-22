/* Handling of signals */

#include <signal.h>
#include <errno.h>
#include <pthread-l4.h>
#include <l4/sys/thread.h>
#include "internals.h"

// In case no backend is available
// TODO: Why is the backend not linked in with musl?!
// Mhm so somehow the weak override by the function from the lib does not work.
// Maybe also broken on uclibc?! Check!
/*
__attribute__((weak))
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
  (void)how;
  (void)set;
  (void)oldset;
  errno = ENOSYS;
  return -1;
}
*/


int pthread_sigmask(int how, const sigset_t * newmask, sigset_t * oldmask)
{
  if (sigprocmask(how, newmask, oldmask) == -1)
    return errno;
  else
    return 0;
}

int pthread_kill(pthread_t thread, int signo);
int pthread_kill(pthread_t thread, int signo)
{
  l4_cap_idx_t c = pthread_l4_cap(thread);

  if (signo >= _NSIG)
    {
      errno = EINVAL;
      return -1;
    }

  if (l4_is_invalid_cap(c))
    {
      errno = ESRCH;
      return -1;
    }

  int x = l4_error(l4_thread_ex_regs(c, ~0UL, ~0UL,
                                     L4_THREAD_EX_REGS_TRIGGER_EXCEPTION));
  if (x)
    {
      errno = EINVAL;
      return -1;
    }

  return x ? -1 : 0;
}
