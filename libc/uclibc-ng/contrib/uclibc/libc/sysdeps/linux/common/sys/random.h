/* Copyright (C) 2015 Bernhard Reutner-Fischer
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
*/

#ifndef	_SYS_RANDOM_H
#define	_SYS_RANDOM_H	1

#include <features.h>
#include <stddef.h>

__BEGIN_DECLS

#include <bits/types.h>

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
#endif

#if 1 // defined __UCLIBC_LINUX_SPECIFIC__
# if 0 /*def __ASSUME_GETRANDOM_SYSCALL */
#  include <linux/random.h>
# else
#  undef GRND_NONBLOCK
#  undef GRND_RANDOM
/*
 * Flags for getrandom(2)
 *
 * GRND_NONBLOCK	Don't block and return EAGAIN instead
 * GRND_RANDOM		Use the /dev/random pool instead of /dev/urandom
 * GRND_INSECURE	Write random data that may not be cryptographically secure.
 */
#  define GRND_NONBLOCK	0x0001
#  define GRND_RANDOM	0x0002
#  define GRND_INSECURE 0x0004
# endif
extern ssize_t getrandom(void *__buf, size_t count, unsigned int flags)
	   __nonnull ((1)) __wur;

/* OpenBSD-compatible access to random bytes.
   May be a cancellation point here, unlike in glibc/musl.  */
# ifndef __getentropy_defined
extern int getentropy(void *__buf, size_t __len) __nonnull ((1)) __wur;
#  define __getentropy_defined
# endif
#endif

__END_DECLS

#endif /* sys/random.h */
