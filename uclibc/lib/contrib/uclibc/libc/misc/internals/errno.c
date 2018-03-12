#include <features.h>
#include <errno.h>
#undef errno

#ifdef __UCLIBC_HAS_TLS__
__thread int errno;
extern __thread int __libc_errno __attribute__ ((alias ("errno"))) attribute_hidden;
#else
extern int errno;
int errno = 0;
# ifdef __UCLIBC_HAS_THREADS__
strong_alias(errno,_errno)
# endif
#endif
