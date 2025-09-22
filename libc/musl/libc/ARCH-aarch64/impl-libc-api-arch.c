#include "pthread_impl.h"

int
ptlc_set_tp(void *tls_tp)
{
  void *tp = (char *)tls_tp + TP_OFFSET;
  __asm __volatile("msr tpidr_el0, %0" : : "r"(tls_tp));
  return 0;
}
