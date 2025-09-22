#include "libc-api.h"
#include "pthread_impl.h"

#include <l4/sys/segment.h>
#include <l4/sys/utcb.h>

int
ptlc_set_tp(void *tls_tp)
{
  __pthread_descr_libc_data(ptlc_tls_tp_to_thread_descr(tls_tp))->self_tp = tls_tp;


  // TODO: Use libc agnostic implementation? This is actually rather a property
  // of our platform.
  // TODO: Consider TP_OFFSET (musl) / TLS_TCB_OFFSET (uclibc)?
  // TODO: Implement for other arches than aarch64
  return fiasco_amd64_set_fs(L4_INVALID_CAP, (l4_umword_t)tls_tp, l4_utcb());
}

// TODO: Assert offset of dtv from tp!
// TODO: Assert offset of sysinfo from tp!
// TODO: Assert offset of canary from tp!