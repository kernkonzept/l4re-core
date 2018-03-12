#include <features.h>
#include <stdlib.h>
#include <unistd.h>
// #include <l4/sys/kdebug.h>

void abort(void)
{
  // enter_kdebug("ABORT");
  _exit(127);
  while (1)
    ;
}
libc_hidden_def(abort)
