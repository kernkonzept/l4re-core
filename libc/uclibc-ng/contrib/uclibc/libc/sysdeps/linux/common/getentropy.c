/*
 * getentropy() by wrapping getrandom(), for µClibc-ng
 *
 * © 2025 mirabilos Ⓕ CC0 or MirBSD or GNU LGPLv2
 *
 * Note: may be a thread cancellation point, unlike the
 * implementations in glibc and musl libc. Should this
 * ever become a concern, it will need patching.
 */

#define _DEFAULT_SOURCE
#include <errno.h>
#include <unistd.h>
#include <sys/random.h>

int
getentropy(void *__buf, size_t __len)
{
	ssize_t n;

	if (__len > 256U) {
		errno = EIO;
		return (-1);
	}

 again:
	if ((n = getrandom(__buf, __len, 0)) == -1)
		switch (errno) {
		case EAGAIN: /* should not happen but better safe than sorry */
		case EINTR:
			goto again;
		default:
			errno = EIO;
			/* FALLTHROUGH */
		case EFAULT:
		case ENOSYS:
			return (-1);
		}
	if ((size_t)n != __len)
		/* also shouldn’t happen (safety net) */
		goto again;
	return (0);
}
