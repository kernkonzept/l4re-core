#include <signal.h>

// L4Re does not implement the tkill/tgkill syscalls that musl's generic raise()
// relies on. Instead the signal delegation is handled by l4re_raise() provided
// by the libc signal backend (see libc_backends), matching the uclibc port.
extern int l4re_raise(int sig);

int raise(int sig)
{
  return l4re_raise(sig);
}
