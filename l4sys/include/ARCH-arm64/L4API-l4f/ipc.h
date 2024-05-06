/**
 * \file
 * \brief   L4 IPC System Calls, ARM
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

EXTERN_C_BEGIN

struct __l4_sys_syscall_res
{
  l4_mword_t tag;
  l4_umword_t label;
};

extern struct __l4_sys_syscall_res
__l4_sys_syscall(l4_mword_t tag,
                 l4_umword_t slabel,
                 l4_umword_t dest,
                 l4_umword_t timeout) L4_NOTHROW;

EXTERN_C_END

L4_INLINE l4_msgtag_t
l4_ipc(l4_cap_idx_t dest, l4_utcb_t *utcb,
       l4_umword_t flags,
       l4_umword_t slabel,
       l4_msgtag_t tag,
       l4_umword_t *rlabel,
       l4_timeout_t timeout) L4_NOTHROW
{
  // No need for memory clobbers. The compiler has to assume that all global
  // data is read/written because __l4_sys_syscall is implemented in a
  // different translation unit.
  struct __l4_sys_syscall_res res
    = __l4_sys_syscall(tag.raw, slabel, dest | flags, timeout.raw);

  (void)utcb;

  if (rlabel)
    *rlabel = res.label;
  tag.raw = res.tag;

  return tag;
}

#endif //__GNUC__
