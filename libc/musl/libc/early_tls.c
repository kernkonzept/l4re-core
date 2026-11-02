/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/*
 * Memory for the TLS of the initial thread, if it does not fit into the
 * builtin TLS buffer. The TLS of the initial thread is set up before the L4Re
 * VFS is initialized, so mmap() cannot be used yet. Get anonymous memory
 * directly from the region manager of the L4Re environment.
 *
 * The memory is never freed.
 */

#include <l4/re/env.h>
#include <l4/re/c/rm.h>
#include <l4/sys/consts.h>

#include <stddef.h>

L4_HIDDEN void *__l4re_early_tls_alloc(void *env, size_t size);

void *__l4re_early_tls_alloc(void *env, size_t size)
{
  l4re_env_t *e = env;
  if (!e)
    return NULL;

  void *addr = 0;
  if (l4re_rm_attach_srv(e->rm, &addr, l4_round_page(size),
                         L4RE_RM_F_SEARCH_ADDR | L4RE_RM_F_RW
                         | L4RE_RM_F_ANONYMOUS | L4RE_RM_F_PRIVATE,
                         L4_INVALID_CAP, 0, L4_PAGESHIFT) < 0)
    return NULL;

  return addr;
}
