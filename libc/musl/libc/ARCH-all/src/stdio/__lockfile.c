#include "stdio_impl.h"

int __lockfile(FILE *f)
{
	pthread_mutex_lock(&f->lock);

	return 1; // return 1, since we need unlock
}

void __unlockfile(FILE *f)
{
	pthread_mutex_unlock(&f->lock);
}
