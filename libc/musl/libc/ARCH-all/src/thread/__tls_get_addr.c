#include "pthread_impl.h"

void *__tls_get_addr(tls_mod_off_t *v)
{
	pthread_descr self = __pthread_thread_self();
	return (void *)(__pthread_descr_libc_data(self)->dtv[v[0]] + v[1]);
}
