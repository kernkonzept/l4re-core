/* Handling of signals */

#include <signal.h>
#include <errno.h>
#include <pthread-l4.h>
#include <l4/sys/thread.h>

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
