/**
 * \internal
 * \file
 * \brief   L4 IPC System Calls, RISC-V
 */
/*
 * Copyright (C) 2021, 2024-2025 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/sys/ipc.h>

#ifdef __GNUC__

#include <l4/sys/compiler.h>

L4_INLINE l4_msgtag_t
l4_ipc(l4_cap_idx_t dest, l4_utcb_t *utcb,
       l4_umword_t flags,
       l4_umword_t slabel,
       l4_msgtag_t tag,
       l4_umword_t *rlabel,
       l4_timeout_t timeout) L4_NOTHROW
{
  register l4_mword_t _tag       __asm__("a0") = tag.raw;
  register l4_umword_t _dest     __asm__("a1") = dest | flags;
  register l4_umword_t _timeout  __asm__("a2") = timeout.raw;
  register l4_umword_t _label    __asm__("a3") = slabel;
  (void)utcb;

  __asm__ __volatile__
    ("ecall"
     :
     "+r" (_tag),
     "+r" (_dest),
     "+r" (_timeout),
     "+r" (_label)
     :
     :
     "memory");

  if (rlabel)
    *rlabel = _label;
  tag.raw = _tag;

  return tag;
}

#endif //__GNUC__
