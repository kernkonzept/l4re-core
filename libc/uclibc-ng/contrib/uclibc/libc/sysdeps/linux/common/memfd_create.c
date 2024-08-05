/*
 * memfd_create() for uClibc-ng
 *
 * Copyright (C) 2024 Waldemar Brodkorb <wbx@uclibc-ng.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/mman.h>
#if defined(__NR_memfd_create)
_syscall2(int, memfd_create, const char *, name, unsigned int, flags)
#endif
