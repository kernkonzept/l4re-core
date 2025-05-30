/**
 * \file
 * Kernel object system calls
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4SYS__KERNEL_OBJECT_H__
#define __L4SYS__KERNEL_OBJECT_H__

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>
#include <l4/sys/utcb.h>

/**
 * \defgroup l4_kernel_object_api Kernel Objects
 * API of kernel objects.
 * \ingroup l4_api
 *
 * \includefile{l4/sys/kernel_object.h}
 */

/**
 * \internal
 * Invoke object, the debugger call.
 * \ingroup l4_kernel_object_api
 * \param obj        The capability of the object to invoke.
 * \param tag        Message tag.
 * \utcb{utcb}
 *
 * \return System call return tag
 */
L4_INLINE l4_msgtag_t
l4_invoke_debugger(l4_cap_idx_t obj, l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW;


/**************************************************************************
 * Implementation
 **************************************************************************/

#include <l4/sys/__kernel_object_impl.h>
#include <l4/sys/ipc.h>

enum L4_kobject_op {
  L4_KOBJECT_OP_DEC_REFCNT = 0,
  L4_KOBJECT_OP_REGISTER_IRQ,
};

L4_INLINE l4_msgtag_t
l4_kobject_dec_refcnt_u(l4_cap_idx_t obj, l4_mword_t diff, l4_utcb_t *u) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_kobject_dec_refcnt(l4_cap_idx_t obj, l4_mword_t diff) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_kobject_dec_refcnt_u(l4_cap_idx_t obj, l4_mword_t diff, l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(u);
  m->mr[0] = L4_KOBJECT_OP_DEC_REFCNT;
  m->mr[1] = diff;
  return l4_ipc_call(obj, u, l4_msgtag(L4_PROTO_KOBJECT, 2, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_kobject_dec_refcnt(l4_cap_idx_t obj, l4_mword_t diff) L4_NOTHROW
{
  return l4_kobject_dec_refcnt_u(obj, diff, l4_utcb());
}

#endif /* ! __L4SYS__KERNEL_OBJECT_H__ */
