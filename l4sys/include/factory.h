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
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

/**
 * \defgroup l4_factory_api Factory
 * \ingroup  l4_kernel_object_api
 *
 * C factory interface to create objects, see L4::Factory for the C++ interface.
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
 * \param         factory     Capability selector for factory to use for
 *                            creation.
 * \param[out]    target_cap  The kernel stores the new task's capability into
 *                            this slot.
 * \param[in,out] utcb_area   Pointer to flexpage that describes an area of
 *                            kernel-user memory that can be used for UTCBs and
 *                            vCPU state-save-areas of the new task.
 *
 *                            On systems without MMU, the flexpage is adjusted
 *                            to reflect the acually allocated physical
 *                            address.
 *
 * \return Syscall return tag.
 *
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 *
 * \note The size of the UTCB area specifies indirectly the number
 *       of UTCBs available for this task. Refer to l4_task_add_ku_mem() for
 *       adding more of this type of memory.
 *
 * \see \ref l4_task_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_task(l4_cap_idx_t factory, l4_cap_idx_t target_cap,
                       l4_fpage_t *utcb_area) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_task_u(l4_cap_idx_t factory, l4_cap_idx_t target_cap,
                         l4_fpage_t *utcb_area, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * Create a new thread.
 *
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new thread's capability into
 *                         this slot.
 *
 * \return Syscall return tag
 *
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 *
 * \see \ref l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_thread(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
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
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 *
 * \note The limit of the new factory is subtracted from the available amount
 *       of the factory used for creation.
 *
 * \note This method is only guaranteed to work with the
 *       \ref l4re_concepts_kernel_factory. For other services, use the
 *       generic L4::Factory::create() method and consult the service documentation
 *       for information on the arguments that need to be passed to the create
 *       stream.
 */
L4_INLINE l4_msgtag_t
l4_factory_create_factory(l4_cap_idx_t factory, l4_cap_idx_t target_cap,
                          unsigned long limit) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
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
 * \param      thread_cap  Optional capability selector of a thread to
 *                         bind the gate to. Use #L4_INVALID_CAP to create
 *                         an unbound IPC gate.
 * \param      label       Optional label of the gate (precisely used if
 *                         `thread_cap` is valid). If `thread_cap` is valid,
 *                         `label` must be present.
 *
 * \return Syscall return tag containing one of the following return codes.
 *
 * \retval L4_EOK      No error occurred.
 * \retval -L4_ENOMEM  Out-of-memory during allocation of the Ipc_gate object.
 * \retval -L4_EINVAL  `thread_cap` is void or points to something that is not
 *                     a thread.
 * \retval -L4_EPERM   No #L4_CAP_FPAGE_S rights on `factory` or `thread_cap`.
 *
 * An unbound IPC gate can be bound to a thread using l4_rcv_ep_bind_thread().
 *
 * \see  l4_kernel_object_gate_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_gate(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap,
                       l4_cap_idx_t thread_cap, l4_umword_t label) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
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
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 *
 * \see \ref l4_irq_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_irq(l4_cap_idx_t factory,
                      l4_cap_idx_t target_cap) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_irq_u(l4_cap_idx_t factory,
                        l4_cap_idx_t target_cap, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * Create a new virtual machine.
 *
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new VM's capability into this
 *                         slot.
 *
 * \return Syscall return tag
 *
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 *
 * \see \ref l4_vm_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_vm(l4_cap_idx_t factory,
                     l4_cap_idx_t target_cap) L4_NOTHROW;

/**
 * \ingroup l4_factory_api
 * Create a new hardware vCPU context. A hardware vCPU context typically
 * represents a hardware vCPU control structure (e.g. VMX VMCS).
 *
 * \param      factory     Capability selector for factory to use for creation.
 * \param[out] target_cap  The kernel stores the new hardware vCPU context's
 *                         capability into this slot.
 *
 * \return Syscall return tag
 *
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 *
 * \see \ref l4_vm_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_vcpu_context(l4_cap_idx_t factory,
                               l4_cap_idx_t target_cap) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_vm_u(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_vcpu_context_u(l4_cap_idx_t factory,
                                 l4_cap_idx_t target_cap,
                                 l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_start_u(long obj, l4_cap_idx_t target,
                          l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE int
l4_factory_create_add_fpage_u(l4_fpage_t d, l4_msgtag_t *tag,
                              l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE int
l4_factory_create_add_int_u(l4_mword_t d, l4_msgtag_t *tag,
                            l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE int
l4_factory_create_add_uint_u(l4_umword_t d, l4_msgtag_t *tag,
                            l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE int
l4_factory_create_add_str_u(char const *s, l4_msgtag_t *tag,
                            l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE int
l4_factory_create_add_lstr_u(char const *s, unsigned len, l4_msgtag_t *tag,
                             l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE int
l4_factory_create_add_nil_u(l4_msgtag_t *tag, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_commit_u(l4_cap_idx_t factory, l4_msgtag_t tag,
                           l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_factory_api
 */
L4_INLINE l4_msgtag_t
l4_factory_create_u(l4_cap_idx_t factory, long obj, l4_cap_idx_t target,
                    l4_utcb_t *utcb) L4_NOTHROW;


/**
 * \ingroup l4_factory_api
 *
 * Create a new object.
 *
 * \param      factory     Factory to use for creation.
 * \param      obj         Protocol ID to describe the type of the object to
 *                         create.
 * \param[out] target      The kernel stores the new objects's capability into
 *                         this slot.
 *
 * \retval L4_EOK     No error occurred.
 * \retval -L4_EPERM  The factory instance requires #L4_CAP_FPAGE_S rights on
 *                    `factory` and #L4_CAP_FPAGE_S is not present.
 * \retval <0         Error code.
 */
L4_INLINE l4_msgtag_t
l4_factory_create(l4_cap_idx_t factory, long obj,
                  l4_cap_idx_t target) L4_NOTHROW;

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_factory_create_task_u(l4_cap_idx_t factory,
                         l4_cap_idx_t target_cap, l4_fpage_t *utcb_area,
                         l4_utcb_t *u) L4_NOTHROW
{
  l4_msgtag_t t;
  t = l4_factory_create_start_u(L4_PROTO_TASK, target_cap, u);
  l4_factory_create_add_fpage_u(*utcb_area, &t, u);
  t = l4_factory_create_commit_u(factory, t, u);
  if (!l4_msgtag_has_error(t))
    {
      l4_msg_regs_t *v = l4_utcb_mr_u(u);
      utcb_area->raw = v->mr[0];
    }
  return t;
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
      v->mr[4] = l4_obj_fpage(thread_cap, 0, L4_CAP_FPAGE_RWS).raw;
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
l4_factory_create_vcpu_context_u(l4_cap_idx_t factory,
                                 l4_cap_idx_t target_cap,
                                 l4_utcb_t *u) L4_NOTHROW
{
  return l4_factory_create_u(factory, L4_PROTO_VCPU_CONTEXT, target_cap, u);
}





L4_INLINE l4_msgtag_t
l4_factory_create_task(l4_cap_idx_t factory,
                       l4_cap_idx_t target_cap, l4_fpage_t *utcb_area) L4_NOTHROW
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
l4_factory_create_vcpu_context(l4_cap_idx_t factory,
                               l4_cap_idx_t target_cap) L4_NOTHROW
{
  return l4_factory_create_vcpu_context_u(factory, target_cap, l4_utcb());
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
  return l4_factory_create_add_lstr_u(s, __builtin_strlen(s) + 1, tag, u);
}

L4_INLINE int
l4_factory_create_add_lstr_u(char const *s, unsigned len, l4_msgtag_t *tag,
                             l4_utcb_t *u) L4_NOTHROW
{

  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  unsigned w = l4_msgtag_words(*tag);
  char *c;
  unsigned i;

  if (w + 1 + l4_bytes_to_mwords(len) > L4_UTCB_GENERIC_DATA_SIZE)
    return 0;

  v->mr[w] = L4_VARG_TYPE_STRING | (len << 16);
  c = (char*)&v->mr[w + 1];
  for (i = 0; i < len; ++i)
    *c++ = *s++;

  w = w + 1 + l4_bytes_to_mwords(len);

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
