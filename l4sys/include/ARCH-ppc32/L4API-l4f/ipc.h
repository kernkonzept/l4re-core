/*!
 * \file
 * \brief   L4 IPC System Calls, PPC
 * \ingroup api_calls
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#ifdef __GNUC__

#include <l4/sys/compiler.h>

#include_next <l4/sys/ipc.h>

#ifndef L4_SYSCALL_MAGIC_OFFSET	
#  define L4_SYSCALL_MAGIC_OFFSET	8
#endif
#define L4_SYSCALL_INVOKE		(-0x00000004-L4_SYSCALL_MAGIC_OFFSET)

L4_INLINE l4_msgtag_t
l4_ipc(l4_cap_idx_t dest, l4_utcb_t *utcb,
       l4_umword_t flags,
       l4_umword_t slabel,
       l4_msgtag_t tag,
       l4_umword_t *rlabel,
       l4_timeout_t timeout) L4_NOTHROW
{
  register l4_umword_t _dest     __asm__("r4") = dest | flags;
  register l4_umword_t _timeout  __asm__("r5") = timeout.raw;
  register l4_mword_t _tag       __asm__("r3") = tag.raw;
  register l4_umword_t _lab      __asm__("r6") = slabel;
  (void)utcb;

  __asm__ __volatile__
    ("bla %[addr]"
     :
     "+r" (_dest),
     "+r" (_timeout),
     "+r" (_lab),
     "+r" (_tag)
     :
     [addr] "i" (L4_SYSCALL_INVOKE)
     :
     "memory", "lr");

  if (rlabel)
    *rlabel = _lab;
  tag.raw = _tag;
  return tag;
}

#endif //__GNUC__
