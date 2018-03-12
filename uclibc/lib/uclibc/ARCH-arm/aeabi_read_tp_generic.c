
#include <l4/sys/utcb.h>
#include <tls.h>

#ifdef NOT_IN_libc
#define HIDDEN __attribute__((visibility("hidden"),nothrow))
#else
#define HIDDEN __attribute__((nothrow))
#endif

unsigned long HIDDEN __aeabi_read_tp(void);
unsigned long HIDDEN __aeabi_read_tp(void)
{
  return l4_utcb_tcr()->user[0] + TLS_PRE_TCB_SIZE;
}
