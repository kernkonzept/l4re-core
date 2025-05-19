#include "stdio_impl.h"
#include <unistd.h>

off_t __stdio_seek(FILE *f, off_t off, int whence)
{
#ifdef NOT_FOR_L4
	return __lseek(f->fd, off, whence);
#else
        return lseek(f->fd, off, whence);
#endif
}
