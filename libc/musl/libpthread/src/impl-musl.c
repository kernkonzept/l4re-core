#include "internals.h"

#include <l4/sys/kdebug.h>

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
