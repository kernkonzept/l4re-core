#include "libc-api.h"
#include "tls.h"

void
ptlc_init_static_tls(void)
{
  __libc_setup_tls(TLS_TCB_SIZE, TLS_TCB_ALIGN);
}

pthread_descr
ptlc_thread_descr_self(void)
{
  return THREAD_SELF;
}

pthread_descr
ptlc_tls_tp_to_thread_descr(void *tls_tp)
{
#if defined(TLS_DTV_AT_TP)
  /* pthread_descr is below TP.  */
  return (pthread_descr)((char *)tls_tp - TLS_PRE_TCB_SIZE);
#elif defined(TLS_TCB_AT_TP)
  return (pthread_descr)tls_tp;
#else
#error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
}

void *
ptlc_thread_descr_to_tls_tp(pthread_descr descr)
{
#if defined(TLS_DTV_AT_TP)
  return (void *)((char *)descr + TLS_PRE_TCB_SIZE);
#elif defined(TLS_TCB_AT_TP)
  return (void *)tls_tp;
#else
#error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
}