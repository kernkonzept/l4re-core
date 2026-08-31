#include "internals.h"

#include <l4/sys/kdebug.h>
#include <stddef.h>

#ifdef TLS_ABOVE_TP
/* The thread pointer points right behind struct pthread. The trailing fields of
 * struct pthread_libc_data (dtv, canary and, on MIPS and RISC-V, the L4 UTCB
 * pointer read by l4_utcb_direct()) are addressed relative to the thread
 * pointer. Thus struct pthread must not contain any padding behind libc_data.
 */
_Static_assert(offsetof(struct pthread, libc_data)
                 + sizeof(struct pthread_libc_data) == sizeof(struct pthread),
               "struct pthread must not have tail padding after libc_data");
#endif

pthread_t __pthread_descr_to_handle(pthread_descr descr)
{
  return descr->p_tid;
}

struct pthread_libc_data *__pthread_libc_data(pthread_t id)
{
  return &handle_to_descr(thread_handle(id))->libc_data;
}

struct pthread_libc_data *__pthread_descr_libc_data(pthread_descr descr)
{
  return &descr->libc_data;
}

// TODO: Maybe instead give musl direct access to `struct pthread`?
size_t __pthread_struct_size(void)
{
  return sizeof(struct pthread);
}
