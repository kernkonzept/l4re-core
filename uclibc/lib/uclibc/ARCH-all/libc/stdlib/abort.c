#include <features.h>
#include <stdlib.h>
#include <unistd.h>
// #include <l4/sys/kdebug.h>

void abort(void)
{
  // enter_kdebug("ABORT");
  _exit(127);
  // _exit() will never return
}
libc_hidden_def(abort)
