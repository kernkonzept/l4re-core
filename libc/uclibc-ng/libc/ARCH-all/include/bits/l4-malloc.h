#pragma once

#include <stddef.h>
#include <sys/cdefs.h>

#ifndef L4_MINIMAL_LIBC

__BEGIN_DECLS
void *uclibc_morecore(long bytes);
__END_DECLS

#endif /* L4_MINIMAL_LIBC */
