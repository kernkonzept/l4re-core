/*
 * Copyright (C) 2018, 2023-2025 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <fcntl.h>
#include <stdio.h>
#include <l4/sys/compiler.h>

int posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
  printf("posix_fadvise(%d, %lld, %lld, %d): void\n",
         fd, (unsigned long long)offset, (unsigned long long)len, advice);
  return 0;
}

#ifndef CONFIG_L4_LIBC_MUSL
L4_STRONG_ALIAS(posix_fadvise, posix_fadvise64)
#endif
