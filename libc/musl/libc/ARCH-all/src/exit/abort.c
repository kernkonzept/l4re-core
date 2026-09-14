#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "atomic.h"

_Noreturn void abort(void)
{
#ifndef L4_MINIMAL_LIBC
  sigset_t sigs, oldsigs;
  struct sigaction act, oldact;

  do
    {
      raise(SIGABRT);

      // We've survived SIGABRT. Either the signal was blocked, ignored or the
      // handler returned. Unblock SIGABRT and restore default disposition.
      // Because another thread could compete, do it in the loop until no
      // signal handler was registered.

      sigemptyset(&sigs);
      sigaddset(&sigs, SIGABRT);
      if (sigprocmask(SIG_UNBLOCK, &sigs, &oldsigs) < 0)
        break;

      memset(&act, 0, sizeof(act));
      act.sa_handler = SIG_DFL;
      sigfillset(&act.sa_mask);
      if (sigaction(SIGABRT, &act, &oldact) < 0)
        break;
    }
  while (sigismember(&oldsigs, SIGABRT) || oldact.sa_handler != SIG_DFL);

  // Effectively unreachable unless another thread flips the SIGABRT
  // disposition repeatedly. Try to raise an exception...
  a_crash();
#endif

  // Still around? Tell the parent and hope that it acts on that...
  _Exit(127);
}
