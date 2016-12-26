/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License 2.  See the file "COPYING-GPL-2" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 2013 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 *
 * This file incorporates work covered by the following copyright notice:
 */

/**
 * \file
 * \brief   L4 IPC System Calls, MIPS
 * \ingroup api_calls
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
  register void       *_utcb     __asm__("s0") = utcb;
  register l4_umword_t _dest     __asm__("s1") = dest | flags;
  register l4_umword_t _timeout  __asm__("s2") = timeout.raw;
  register l4_mword_t  _tag      __asm__("s3") = tag.raw;
  register l4_umword_t _label    __asm__("s4") = slabel;

  /* the kernl preserves sp(29), fp(30), and gp(28) */
  /* s0 - s4 are in/out arguments */
  __asm__ __volatile__
    ("syscall"
     :
     "+r" (_dest),
     "+r" (_timeout),
     "+r" (_label),
     "+r" (_tag),
     "+r" (_utcb)
     :
     : "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11",
       "$12", "$13", "$14", "$15", "$22", "$23", "$24", "$25",
       "$31", "memory");

  if (rlabel)
    *rlabel = _label;
  tag.raw = _tag;

  return tag;
}

#endif //__GNUC__
