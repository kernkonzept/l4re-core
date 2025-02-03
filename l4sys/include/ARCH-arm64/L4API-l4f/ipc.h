/**
 * \file
 * \brief   L4 IPC System Calls, ARM
 * \ingroup l4_api
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

__BEGIN_DECLS

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

__END_DECLS

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
