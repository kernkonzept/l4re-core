/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <bits/l4-malloc.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>

extern char __heap_start[];
extern char __heap_end[];
static char *heap_pos = __heap_start;

void *uclibc_morecore(long bytes)
{
  if (bytes <= 0)
    return heap_pos;

  if (__heap_end - heap_pos < bytes)
    return (void*)-1;

  void *ret = heap_pos;
  heap_pos += bytes;
  return ret;
}
