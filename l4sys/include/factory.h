/**
 * \file
 * \brief Common factory related definitions.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>,
 *               Henning Schild <hschild@os.inf.tu-dresden.de>
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

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

/**
 * \defgroup l4_factory_api Factory
 * \ingroup  l4_kernel_object_api
 *
 * C factory interface to create kernel objects.
 *
 * A factory is used to create all kinds of kernel objects:
 * - \ref l4_task_api
 * - \ref l4_thread_api
 * - \ref l4_factory_api
 * - \ref l4_kernel_object_gate_api
 * - \ref l4_irq_api
 * - \ref l4_vm_api
 *
 * To create a new kernel object the caller has to specify the factory to use
 * for creation. The caller has to allocate a capability slot where the kernel
 * stores the new object's capability.
 *
 * The factory is equipped with a limit that limits the amount of kernel
 * memory available for that factory.
 *
 * \note The limit does not give any guarantee for the amount of available
 *       kernel memory.
 *
 * \includefile{l4/sys/factory.h}
 *
 * For the C++ interface refer to L4::Factory.
 */

/**
 * \defgroup l4_vm_api Virtual Machines
 * \ingroup l4_kernel_object_api
 * \brief Virtual Machine API
 */

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_task
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new task's capability into
 *                         this slot.
 * \param      utcb_area   Flexpage that describes an area of kernel-user
 *                         memory that can be used for UTCBs and vCPU
 *                         state-save-areas of the new task.
 *
 * \return Syscall return tag.
 *
 * \note The size of the UTCB area specifies indirectly the number
 *       of UTCBs available for this task. Refer to L4::Task::add_ku_mem
 *       / l4_task_add_ku_mem() for adding more of this type of memory.
 *
 * \see \ref l4_task_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_task(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap, l4_fpage_t const utcb_area) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_task
 * \param factory  Capability selector for factory to use for creation.
 * \copydetails L4::Factory::create_task
 */
L4_INLINE l4_msgtag_t
l4_factory_create_task_u(l4_cap_idx_t factory, l4_cap_idx_t target_cap,
                         l4_fpage_t const utcb_area, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_thread
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new thread's capability into
 *                         this slot.
 *
 * \return Syscall return tag
 *
 * \see \ref l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_thread(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_thread
 * \param factory  Capability selector for factory to use for creation.
 * \copydetails  L4::Factory::create_thread
 */
L4_INLINE l4_msgtag_t
l4_factory_create_thread_u(l4_cap_idx_t factory,
                           l4_cap_idx_t target_cap, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_factory
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new factory's capability into
 *                         this slot.
 * \param      limit       Limit for the new factory in bytes.
 *
 * \return Syscall return tag
 *
 * \note The limit of the new factory is subtracted from the available amount
 *       of the factory used for creation.
 */
L4_INLINE l4_msgtag_t
l4_factory_create_factory(l4_cap_idx_t factory, l4_cap_idx_t target_cap,
                          unsigned long limit) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_factory
 * \param factory  Capability selector for factory to use for creation.
 * \copydetails L4::Factory::create_factory
 */
L4_INLINE l4_msgtag_t
l4_factory_create_factory_u(l4_cap_idx_t factory, l4_cap_idx_t target_cap,
                            unsigned long limit, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_gate
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new IPC gate's capability into
 *                         this slot.
 * \param      thread_cap  Optional capability selector of the thread to
 *                         bind the gate to. Use #L4_INVALID_CAP to create
 *                         an unbound IPC gate.
 * \param      label       Optional label of the gate (is used if
 *                         `thread_cap` is valid).
 *
 * \return Syscall return tag containing one of the following return codes.
 *
 * \retval L4_EOK      No error occurred.
 * \retval -L4_ENOMEM  Out-of-memory during allocation of the Ipc_gate object.
 * \retval -L4_ENOENT  `thread_cap` is void or points to something that is not
 *                     a thread.
 * \retval -L4_EPERM   No write rights on `thread_cap`.
 *
 * An unbound IPC gate can be bound to a thread using #l4_ipc_gate_bind_thread.
 *
 * \see  l4_kernel_object_gate_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_gate(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap,
                       l4_cap_idx_t thread_cap, l4_umword_t label) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_gate
 * \param factory  Capability selector for factory to use for creation.
 * \copydetails  L4::Factory::create_gate
 */
L4_INLINE l4_msgtag_t
l4_factory_create_gate_u(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap,
                         l4_cap_idx_t thread_cap, l4_umword_t label,
                         l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * Create a new IRQ sender.
 *
 * \param      factory     Factory to use for creation.
 * \param[out] target_cap  The kernel stores the new IRQ's capability into this
 *                         slot.
 *
 * \return Syscall return tag
 *
 * \see \ref l4_irq_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_irq(l4_cap_idx_t factory,
                      l4_cap_idx_t target_cap) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_irq
 * \param factory  Factory to use for creation.
 * \copydetails L4::Factory::create_irq
 */
L4_INLINE l4_msgtag_t
l4_factory_create_irq_u(l4_cap_idx_t factory,
                        l4_cap_idx_t target_cap, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_vm
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new VM's capability into this
 *                         slot.
 *
 * \return Syscall return tag
 *
 * \see \ref l4_vm_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_vm(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap) L4_NOTHROW;
/**
 * \ingroup l4_factory_api
 * \copybrief L4::Factory::create_vm
 * \param factory  Capability selector for factory to use for creation.
 * \copydetails L4::Factory::create_vm
 */
L4_INLINE l4_msgtag_t
l4_factory_create_vm_u(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap, l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_factory_create_start_u(long obj, l4_cap_idx_t target,
                          l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE int
l4_factory_create_add_fpage_u(l4_fpage_t d, l4_msgtag_t *tag,
                              l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE int
l4_factory_create_add_int_u(l4_mword_t d, l4_msgtag_t *tag,
                            l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE int
l4_factory_create_add_uint_u(l4_umword_t d, l4_msgtag_t *tag,
                            l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE int
l4_factory_create_add_str_u(char const *s, l4_msgtag_t *tag,
                            l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE int
l4_factory_create_add_lstr_u(char const *s, int len, l4_msgtag_t *tag,
                             l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE int
l4_factory_create_add_nil_u(l4_msgtag_t *tag, l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_factory_create_commit_u(l4_cap_idx_t factory, l4_msgtag_t tag,
                           l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_factory_create_u(l4_cap_idx_t factory, long obj, l4_cap_idx_t target,
                    l4_utcb_t *utcb) L4_NOTHROW;


L4_INLINE l4_msgtag_t
l4_factory_create(l4_cap_idx_t factory, long obj,
                  l4_cap_idx_t target) L4_NOTHROW;

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_factory_create_task_u(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap, l4_fpage_t utcb_area,
                         l4_utcb_t *u) L4_NOTHROW
{
  l4_msgtag_t t;
  t = l4_factory_create_start_u(L4_PROTO_TASK, target_cap, u);
  l4_factory_create_add_fpage_u(utcb_area, &t, u);
  return l4_factory_create_commit_u(factory, t, u);
}

L4_INLINE l4_msgtag_t
l4_factory_create_thread_u(l4_cap_idx_t factory,
                           l4_cap_idx_t target_cap, l4_utcb_t *u) L4_NOTHROW
{
  return l4_factory_create_u(factory, L4_PROTO_THREAD, target_cap, u);
}

L4_INLINE l4_msgtag_t
l4_factory_create_factory_u(l4_cap_idx_t factory,
                            l4_cap_idx_t target_cap, unsigned long limit,
                            l4_utcb_t *u) L4_NOTHROW
{
  l4_msgtag_t t;
  t = l4_factory_create_start_u(L4_PROTO_FACTORY, target_cap, u);
  l4_factory_create_add_uint_u(limit, &t, u);
  return l4_factory_create_commit_u(factory, t, u);
}

L4_INLINE l4_msgtag_t
l4_factory_create_gate_u(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap,
                         l4_cap_idx_t thread_cap, l4_umword_t label,
                         l4_utcb_t *u) L4_NOTHROW
{
  l4_msgtag_t t;
  l4_msg_regs_t *v;
  int items = 0;
  t = l4_factory_create_start_u(0, target_cap, u);
  l4_factory_create_add_uint_u(label, &t, u);
  v = l4_utcb_mr_u(u);
  if (!(thread_cap & L4_INVALID_CAP_BIT))
    {
      items = 1;
      v->mr[3] = l4_map_obj_control(0,0);
      v->mr[4] = l4_obj_fpage(thread_cap, 0, L4_FPAGE_RWX).raw;
    }
  t = l4_msgtag(l4_msgtag_label(t), l4_msgtag_words(t), items, l4_msgtag_flags(t));
  return l4_factory_create_commit_u(factory, t, u);
}

L4_INLINE l4_msgtag_t
l4_factory_create_irq_u(l4_cap_idx_t factory,
                        l4_cap_idx_t target_cap, l4_utcb_t *u) L4_NOTHROW
{
  return l4_factory_create_u(factory, L4_PROTO_IRQ_SENDER, target_cap, u);
}

L4_INLINE l4_msgtag_t
l4_factory_create_vm_u(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap,
                       l4_utcb_t *u) L4_NOTHROW
{
  return l4_factory_create_u(factory, L4_PROTO_VM, target_cap, u);
}





L4_INLINE l4_msgtag_t
l4_factory_create_task(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap, l4_fpage_t const utcb_area) L4_NOTHROW
{
  return l4_factory_create_task_u(factory, target_cap, utcb_area, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_factory_create_thread(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap) L4_NOTHROW
{
  return l4_factory_create_thread_u(factory, target_cap, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_factory_create_factory(l4_cap_idx_t factory,
                          l4_cap_idx_t target_cap, unsigned long limit) L4_NOTHROW

{
  return l4_factory_create_factory_u(factory, target_cap, limit, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_factory_create_gate(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap,
                       l4_cap_idx_t thread_cap, l4_umword_t label) L4_NOTHROW
{
  return l4_factory_create_gate_u(factory, target_cap, thread_cap, label, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_factory_create_irq(l4_cap_idx_t factory,
                      l4_cap_idx_t target_cap) L4_NOTHROW
{
  return l4_factory_create_irq_u(factory, target_cap, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_factory_create_vm(l4_cap_idx_t factory,
                     l4_cap_idx_t target_cap) L4_NOTHROW
{
  return l4_factory_create_vm_u(factory, target_cap, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_factory_create_start_u(long obj, l4_cap_idx_t target_cap,
                          l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  l4_buf_regs_t *b = l4_utcb_br_u(u);
  v->mr[0] = obj;
  b->bdr = 0;
  b->br[0] = target_cap | L4_RCV_ITEM_SINGLE_CAP;
  return l4_msgtag(L4_PROTO_FACTORY, 1, 0, 0);
}

L4_INLINE int
l4_factory_create_add_fpage_u(l4_fpage_t d, l4_msgtag_t *tag,
                              l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  int w = l4_msgtag_words(*tag);
  if (w + 2 > L4_UTCB_GENERIC_DATA_SIZE)
    return 0;
  v->mr[w] = L4_VARG_TYPE_FPAGE | (sizeof(l4_fpage_t) << 16);
  v->mr[w + 1] = d.raw;
  w += 2;
  tag->raw = (tag->raw & ~0x3fUL) | (w & 0x3f);
  return 1;
}

L4_INLINE int
l4_factory_create_add_int_u(l4_mword_t d, l4_msgtag_t *tag,
                            l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  int w = l4_msgtag_words(*tag);
  if (w + 2 > L4_UTCB_GENERIC_DATA_SIZE)
    return 0;
  v->mr[w] = L4_VARG_TYPE_MWORD | (sizeof(l4_mword_t) << 16);
  v->mr[w + 1] = d;
  w += 2;
  tag->raw = (tag->raw & ~0x3fUL) | (w & 0x3f);
  return 1;
}

L4_INLINE int
l4_factory_create_add_uint_u(l4_umword_t d, l4_msgtag_t *tag,
                             l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  int w = l4_msgtag_words(*tag);
  if (w + 2 > L4_UTCB_GENERIC_DATA_SIZE)
    return 0;
  v->mr[w] = L4_VARG_TYPE_UMWORD | (sizeof(l4_umword_t) << 16);
  v->mr[w + 1] = d;
  w += 2;
  tag->raw = (tag->raw & ~0x3fUL) | (w & 0x3f);
  return 1;
}

L4_INLINE int
l4_factory_create_add_str_u(char const *s, l4_msgtag_t *tag,
                            l4_utcb_t *u) L4_NOTHROW
{
  return l4_factory_create_add_lstr_u(s, __builtin_strlen(s), tag, u);
}

L4_INLINE int
l4_factory_create_add_lstr_u(char const *s, int len, l4_msgtag_t *tag,
                             l4_utcb_t *u) L4_NOTHROW
{

  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  int w = l4_msgtag_words(*tag);
  char *c;
  int i;

  if (w + 1 + (len + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t)
      > L4_UTCB_GENERIC_DATA_SIZE)
    return 0;

  v->mr[w] = L4_VARG_TYPE_STRING | (len << 16);
  c = (char*)&v->mr[w + 1];
  for (i = 0; i < len; ++i)
    *c++ = *s++;

  w = w + 1 + (len + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t);

  tag->raw = (tag->raw & ~0x3fUL) | (w & 0x3f);
  return 1;
}

L4_INLINE int
l4_factory_create_add_nil_u(l4_msgtag_t *tag, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  int w = l4_msgtag_words(*tag);
  v->mr[w] = L4_VARG_TYPE_NIL;
  ++w;
  tag->raw = (tag->raw & ~0x3fUL) | (w & 0x3f);
  return 1;
}


L4_INLINE l4_msgtag_t
l4_factory_create_commit_u(l4_cap_idx_t factory, l4_msgtag_t tag,
                           l4_utcb_t *u) L4_NOTHROW
{
  return l4_ipc_call(factory, u, tag, L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_factory_create_u(l4_cap_idx_t factory, long obj, l4_cap_idx_t target,
                    l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t t = l4_factory_create_start_u(obj, target, utcb);
  return l4_factory_create_commit_u(factory, t, utcb);
}


L4_INLINE l4_msgtag_t
l4_factory_create(l4_cap_idx_t factory, long obj,
                  l4_cap_idx_t target) L4_NOTHROW
{
  return l4_factory_create_u(factory, obj, target, l4_utcb());
}
