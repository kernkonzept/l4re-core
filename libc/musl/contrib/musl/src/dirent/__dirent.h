#include "lock.h"

struct __dirstream
{
	off_t tell;
	int fd;
	int buf_pos;
	int buf_end;
#ifdef NOT_FOR_L4
	volatile int lock[1];
#else
	libc_lock_t lock;
#endif
	/* Any changes to this struct must preserve the property:
	 * offsetof(struct __dirent, buf) % sizeof(off_t) == 0 */
	char buf[2048];
};
