/**
 * \file
 * \brief  Cache functions
 */
/* SPDX-License-Identifier: ((GPL-2.0-only WITH mif-exception) OR LicenseRef-kk-custom) */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#pragma once

#include_next <l4/sys/cache.h>
#include <l4/sys/ipc.h>
#include <l4/sys/consts.h>
#include <l4/sys/compiler.h>

L4_INLINE int
l4_cache_clean_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  (void)start;(void)end;
  return 0;
}

L4_INLINE int
l4_cache_flush_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  (void)start;(void)end;
  return 0;
}

L4_INLINE int
l4_cache_inv_data(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  (void)start;(void)end;
  return 0;
}

L4_INLINE int
l4_cache_coherent(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  (void)start;(void)end;
  asm volatile ("fence.i");
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent(unsigned long start,
                      unsigned long end) L4_NOTHROW
{
  (void)start;(void)end;
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent_full(void) L4_NOTHROW
{
  return 0;
}
