
// prototype from
// pkg/uclibc/lib/contrib/uclibc/libc/sysdeps/linux/common/bits/sigthread.h

#pragma once

#if !defined _SIGNAL_H && !defined _PTHREAD_H
# error "Never include this file directly.  Use <pthread.h> instead"
#endif

/* Send signal SIGNO to the given thread. */
extern int pthread_kill (pthread_t __threadid, int __signo) __THROW;
