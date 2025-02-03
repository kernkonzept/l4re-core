/**
 * \internal
 * \file
 * \brief   L4 IPC System Calls, ARM
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/sys/ipc.h>

#ifdef __GNUC__

#include <l4/sys/compiler.h>
#include <l4/sys/syscall_defs.h>

L4_INLINE l4_msgtag_t
l4_ipc(l4_cap_idx_t dest, l4_utcb_t *utcb,
       l4_umword_t flags,
       l4_umword_t slabel,
       l4_msgtag_t tag,
       l4_umword_t *rlabel,
       l4_timeout_t timeout) L4_NOTHROW
{
  register l4_umword_t _dest     __asm__("r2") = dest | flags;
  register l4_umword_t _timeout  __asm__("r3") = timeout.raw;
  register l4_mword_t _tag       __asm__("r0") = tag.raw;
  register l4_umword_t _label    __asm__("r4") = slabel;
  (void)utcb;

  __asm__ __volatile__
    ("mov r5, %[sc]         \n"
     "blx __l4_sys_syscall  \n"
     :
     "+r" (_dest),
     "+r" (_timeout),
     "+r" (_label),
     "+r" (_tag)
     :
     [sc] "i" (L4_SYSCALL_INVOKE)
     :
     "cc", "memory", "r5", "ip", "lr");

  if (rlabel)
    *rlabel = _label;
  tag.raw = _tag;

  return tag;
}

#endif //__GNUC__
