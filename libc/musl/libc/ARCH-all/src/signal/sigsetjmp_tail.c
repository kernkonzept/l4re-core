#include <setjmp.h>
#include <signal.h>

hidden int __sigsetjmp_tail(sigjmp_buf jb, int ret)
{
  void *p = jb->__ss;
  sigprocmask(SIG_SETMASK, ret ? p : 0, ret ? 0 : p);
  return ret;
}
