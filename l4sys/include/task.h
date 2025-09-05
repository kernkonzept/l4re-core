/**
 * \file
 * Common task related definitions.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once
#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

/**
 * \defgroup l4_task_api Task
 * \ingroup  l4_kernel_object_api
 * C interface of the Task kernel object, see L4::Task for the C++ interface.
 *
 * A task represents a combination of the address spaces provided
 * by the L4Re micro kernel. A task consists of at least a memory address space
 * and an object address space. On IA32 there is also an IO-port address space.
 *
 * Task objects are created using the \ref l4_factory_api interface.
 *
 * \includefile{l4/sys/task.h}
 */

/**
 * Map resources available in the source task to a destination task.
 * \ingroup l4_task_api
 *
 * \param dst_task   Capability selector of the destination task.
 * \param src_task   Capability selector of the source task.
 * \param snd_fpage  Send flexpage that describes an area in the address
 *                   space or object space of the source task.
 * \param snd_base   Send base that describes an offset in the receive window
 *                   of the destination task. The lower bits contain additional
 *                   map control flags (see #l4_fpage_cacheability_opt_t for
 *                   memory mappings, #L4_obj_fpage_ctl for object mappings,
 *                   and #L4_MAP_ITEM_GRANT; also see l4_map_control() and
 *                   l4_map_obj_control()).
 *
 * \return Syscall return tag. The function l4_error() shall be used to test
 *         if the map operation was successful.
 *
 * \retval L4_EOK      Operation successful (but see notes below).
 * \retval -L4_EPERM   Insufficient permissions; see precondition.
 * \retval -L4_EINVAL  Invalid source task capability.
 * \retval -L4_IPC_SEMAPFAILED The map operation failed due to limited quota.
 *
 * \pre The capability `dst_task` must have the permission #L4_CAP_FPAGE_W.
 *
 * This method allows for asynchronous transfer of capabilities, memory
 * mappings, and IO-port mappings (on IA32) from one task to another.
 * The receive window is the whole address space of `dst_task`. By specifying
 * proper rights in `snd_fpage` and `snd_base`, it is possible to remove rights
 * during transfer.
 *
 * \note If the send flexpage is of type #L4_FPAGE_OBJ, the #L4_CAP_FPAGE_S
 *       right is removed from the transferred capability unless both the
 *       source and destination task capabilities possess the #L4_CAP_FPAGE_S
 *       right themselves.
 *
 * \note Even with l4_error() returning L4_EOK there might be cases where not
 *       all pages of the send flexpage were mapped respectively granted to
 *       the destination task, for instance, if the corresponding mapping in
 *       the destination task does already exist.
 *
 * For more information on spaces and mappings, see
 * \ref l4re_concepts_mapping. The flexpage API is described in more detail at
 * \ref l4_fpage_api.
 *
 * \note For peculiarities when using grant, see #L4_MAP_ITEM_GRANT.
 */
L4_INLINE l4_msgtag_t
l4_task_map(l4_cap_idx_t dst_task, l4_cap_idx_t src_task,
            l4_fpage_t snd_fpage, l4_umword_t snd_base) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_map_u(l4_cap_idx_t dst_task, l4_cap_idx_t src_task,
              l4_fpage_t snd_fpage, l4_umword_t snd_base, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Revoke rights from the task.
 * \ingroup l4_task_api
 *
 * \param task          Capability selector of destination task
 * \param fpage         Flexpage that describes an area in one capability space
 *                      of the destination task and the rights to revoke.
 * \param map_mask      Unmap mask, see #l4_unmap_flags_t
 *
 * \return Syscall return tag
 *
 * This method allows to revoke rights from the destination task. The rights to
 * revoke are specified in the flexpage, see l4_fpage_rights(). For a flexpage
 * describing IO ports or memory, it also revokes rights from all the tasks
 * that got the rights delegated from the destination task (i.e., this operation
 * does a recursive rights revocation). The capability is unmapped if certain
 * rights are specified, see below for details. It is guaranteed that the rights
 * revocation is completed before this function returns.
 *
 * Note that this function cannot be used to revoke the reference counting
 * permission (see #L4_FPAGE_C_REF_CNT) or the IPC-gate server permission
 * (see #L4_FPAGE_C_IPCGATE_SVR) from object capabilities.
 *
 * It depends on the platform and the object type which rights need to be
 * specified in the `rights` field of `fpage` to unmap a capability:
 *  - An object capability is unmapped if and only if the #L4_CAP_FPAGE_R right
 *    bit is set.
 *  - An IO port is unmapped if and only if any right bit is set.
 *  - Memory is unmapped if and only if the #L4_FPAGE_RO right bit is set.
 *
 * \note Depending on the page-table features supported by the hardware,
 *       revocation of certain rights from a memory capability can be a no-op
 *       (i.e., the rights are not revoked). Further, revocation of certain
 *       rights may grant other rights which were not present before. For
 *       instance, on an architecture without support for NX, revoking X does
 *       nothing. For another example, revoking only X from an execute-only page
 *       grants read permission (because the mapping remains present in the page
 *       table).
 *
 * \note If the reference counter of a kernel object referenced in `fpage`
 *       goes down to zero (as a result of deleting capabilities), the deletion
 *       of the object is initiated. Objects are not destroyed until all other
 *       kernel objects holding a reference to it drop the reference.
 */
L4_INLINE l4_msgtag_t
l4_task_unmap(l4_cap_idx_t task, l4_fpage_t fpage,
              l4_umword_t map_mask) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_unmap_u(l4_cap_idx_t task, l4_fpage_t fpage,
                l4_umword_t map_mask, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Revoke rights from a task.
 * \ingroup l4_task_api
 *
 * \param task          Capability selector of destination task
 * \param fpages        An array of flexpages. Each item describes an area in
 *                      one capability space of the destination task.
 * \param num_fpages    The size of the fpages array in elements (number of
 *                      fpages sent).
 * \param map_mask      Unmap mask, see #l4_unmap_flags_t
 *
 * \return Syscall return tag
 *
 * Revoke rights specified in an array of flexpages, see #l4_task_unmap for
 * details.
 *
 * \pre The caller needs to take care that num_fpages is not bigger
 *      than L4_UTCB_GENERIC_DATA_SIZE - 2.
 */
L4_INLINE l4_msgtag_t
l4_task_unmap_batch(l4_cap_idx_t task, l4_fpage_t const *fpages,
                    unsigned num_fpages, l4_umword_t map_mask) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_unmap_batch_u(l4_cap_idx_t task, l4_fpage_t const *fpages,
                      unsigned num_fpages, l4_umword_t map_mask,
                      l4_utcb_t *u) L4_NOTHROW;

/**
 * Release capability and delete object.
 * \ingroup l4_task_api
 *
 * \param task  Capability selector of destination task.
 * \param obj   Capability index of the object to delete.
 *
 * \return Syscall return tag
 *
 * If `obj` has the delete permission, initiates the deletion of the object.
 * This implies that all capabilities for that object are gone afterwards.
 * However, kernel-internally, objects are not destroyed until all other
 * kernel objects holding a reference to it drop the reference. Hence, quota
 * used by that object might not be freed immediately.
 *
 * If `obj` does not have the delete permission, no error will be reported and
 * only the capability `obj` is removed. (Note that, depending on the object’s
 * reference counter, this might still imply initiation of deletion.)
 *
 * This operation is equivalent to l4_task_unmap() with #L4_FP_DELETE_OBJ flag.
 */
L4_INLINE l4_msgtag_t
l4_task_delete_obj(l4_cap_idx_t task, l4_cap_idx_t obj) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_delete_obj_u(l4_cap_idx_t task, l4_cap_idx_t obj,
                     l4_utcb_t *u) L4_NOTHROW;

/**
 * Release object capability.
 * \ingroup l4_task_api
 *
 * \param task          Capability selector of destination task
 * \param cap           Capability selector of object to release
 *
 * \return Syscall return tag
 *
 * This operation unmaps the capability from the specified task. This operation
 * is equivalent to unmapping a single object capability by specifying all
 * object rights as unmap mask.
 *
 * \note If the reference counter of the kernel object referenced by `cap`
 *       goes down to zero, deletion of the object is initiated. Objects are not
 *       destroyed until all other kernel objects holding a reference to it drop
 *       the reference.
 */
L4_INLINE l4_msgtag_t
l4_task_release_cap(l4_cap_idx_t task, l4_cap_idx_t cap) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_release_cap_u(l4_cap_idx_t task, l4_cap_idx_t cap,
                      l4_utcb_t *u) L4_NOTHROW;


/**
 * Check whether a capability is present (refers to an object).
 * \ingroup l4_task_api
 *
 * \param task  Task to check the capability in.
 * \param cap   Valid capability to check for presence.
 *
 * \retval "l4_msgtag_t::label() > 0"   Capability is present (refers to an
 *                                      object).
 * \retval "l4_msgtag_t::label() == 0"  No capability present (void object).
 *
 * A capability is considered present when it refers to an existing
 * kernel object.
 *
 * \pre `cap` must be a valid capability index (i.e. not L4_INVALID_CAP or
 *      the like).
 */
L4_INLINE l4_msgtag_t
l4_task_cap_valid(l4_cap_idx_t task, l4_cap_idx_t cap) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_cap_valid_u(l4_cap_idx_t task, l4_cap_idx_t cap, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Test whether two capabilities point to the same object with the same
 * permissions (only considering selected permissions).
 * \ingroup l4_task_api
 *
 * \param task   Capability selector for the destination task to do the lookup
 *               in.
 * \param cap_a  Capability selector for the first capability to compare.
 * \param cap_b  Capability selector for the second capability to compare.
 *
 * \retval "l4_msgtag_t::label() = 1"  The compared capabilities point to the
 *                                     same object with same considered
 *                                     permission.
 * \retval "l4_msgtag_t::label() = 0"  The compared capabilities do **not**
 *                                     point to the same object or differ in
 *                                     the considered permission.
 *
 * - For L4::Ipc_gate objects, only the permissions #L4_CAP_FPAGE_W,
 *   #L4_CAP_FPAGE_S, and #L4_FPAGE_C_OBJ_RIGHT1 are considered for the
 *   comparison. Differences in other permissions are ignored.
 * - For other objects, only the permissions #L4_CAP_FPAGE_W and
 *   #L4_CAP_FPAGE_S are considered for the comparison. Differences in other
 *   permissions are ignored.
 *
 * Note that having the #L4_CAP_FPAGE_R permission is implicit in possessing
 * the capability.
 */
L4_INLINE l4_msgtag_t
l4_task_cap_equal(l4_cap_idx_t task, l4_cap_idx_t cap_a,
                  l4_cap_idx_t cap_b) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_add_ku_mem_u(l4_cap_idx_t task, l4_fpage_t *ku_mem,
                     l4_utcb_t *u) L4_NOTHROW;

/**
 * Add kernel-user memory.
 * \ingroup l4_task_api
 *
 * \param task            Capability selector of the task to add the memory to.
 * \param[in,out] ku_mem  Flexpage describing the virtual area the memory goes
 *                        to. On systems without MMU, the flexpage is adjusted
 *                        to reflect the acually allocated physical address.
 *
 * \return Syscall return tag
 *
 * Kernel-user memory (ku_mem) is memory that is shared between the kernel and
 * user-space. It is needed for the UTCB area of threads (see
 * l4_thread_control_bind()) and for (extended) vCPU state. Note that existing
 * kernel-user memory cannot be unmapped or mapped somewhere else.
 *
 * \note The amount of kernel-user memory that can be allocated at once is
 *       limited by the used kernel implementation. The minimum allocatable
 *       amount is one page (`L4_PAGESIZE`). A portable implementation should
 *       not depend on allocations greater than 16KiB to succeed.
 *
 * \note This function is only guaranteed to work on L4::Task objects. It
 *       might or might not work on L4::Vm objects or on L4Re::Dma_space
 *       objects but there is no practical use for adding kernel-user memory
 *       to L4::Vm objects or to L4Re::Dma_space objects.
 */
L4_INLINE l4_msgtag_t
l4_task_add_ku_mem(l4_cap_idx_t task, l4_fpage_t *ku_mem) L4_NOTHROW;


/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_task_cap_equal_u(l4_cap_idx_t task, l4_cap_idx_t cap_a,
                    l4_cap_idx_t cap_b, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Operations on task objects.
 * \ingroup l4_protocol_ops
 */
enum L4_task_ops
{
  L4_TASK_MAP_OP           = 0UL,    /**< Map */
  L4_TASK_UNMAP_OP         = 1UL,    /**< Unmap */
  L4_TASK_CAP_INFO_OP      = 2UL,    /**< Cap info */
  L4_TASK_ADD_KU_MEM_OP    = 3UL,    /**< Add kernel-user memory */
  L4_TASK_LDT_SET_X86_OP   = 0x11UL, /**< x86: LDT set */
  L4_TASK_MAP_VGICC_ARM_OP = 0x12UL, /**< Arm: Map virtual GICC area */
};


/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>


L4_INLINE l4_msgtag_t
l4_task_map_u(l4_cap_idx_t dst_task, l4_cap_idx_t src_task,
              l4_fpage_t snd_fpage, l4_umword_t snd_base, l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  v->mr[0] = L4_TASK_MAP_OP;
  v->mr[3] = l4_map_obj_control(0,0);
  v->mr[4] = l4_obj_fpage(src_task, 0, L4_CAP_FPAGE_RWS).raw;
  v->mr[1] = snd_base;
  v->mr[2] = snd_fpage.raw;
  return l4_ipc_call(dst_task, u, l4_msgtag(L4_PROTO_TASK, 3, 1, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_task_unmap_u(l4_cap_idx_t task, l4_fpage_t fpage,
                l4_umword_t map_mask, l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  v->mr[0] = L4_TASK_UNMAP_OP;
  v->mr[1] = map_mask;
  v->mr[2] = fpage.raw;
  return l4_ipc_call(task, u, l4_msgtag(L4_PROTO_TASK, 3, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_task_unmap_batch_u(l4_cap_idx_t task, l4_fpage_t const *fpages,
                      unsigned num_fpages, l4_umword_t map_mask,
                      l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  v->mr[0] = L4_TASK_UNMAP_OP;
  v->mr[1] = map_mask;
  __builtin_memcpy(&v->mr[2], fpages, num_fpages * sizeof(l4_fpage_t));
  return l4_ipc_call(task, u, l4_msgtag(L4_PROTO_TASK, 2 + num_fpages, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_task_cap_valid_u(l4_cap_idx_t task, l4_cap_idx_t cap, l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  v->mr[0] = L4_TASK_CAP_INFO_OP;
  v->mr[1] = cap & ~1UL;
  return l4_ipc_call(task, u, l4_msgtag(L4_PROTO_TASK, 2, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_task_cap_equal_u(l4_cap_idx_t task, l4_cap_idx_t cap_a,
                    l4_cap_idx_t cap_b, l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  v->mr[0] = L4_TASK_CAP_INFO_OP;
  v->mr[1] = cap_a;
  v->mr[2] = cap_b;
  return l4_ipc_call(task, u, l4_msgtag(L4_PROTO_TASK, 3, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_task_add_ku_mem_u(l4_cap_idx_t task, l4_fpage_t *ku_mem,
                     l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  l4_msgtag_t ret;
  v->mr[0] = L4_TASK_ADD_KU_MEM_OP;
  v->mr[1] = ku_mem->raw;
  ret = l4_ipc_call(task, u, l4_msgtag(L4_PROTO_TASK, 2, 0, 0), L4_IPC_NEVER);
  if (!l4_msgtag_has_error(ret))
    {
      l4_msg_regs_t *v = l4_utcb_mr_u(u);
      ku_mem->raw = v->mr[0];
    }
  return ret;
}



L4_INLINE l4_msgtag_t
l4_task_map(l4_cap_idx_t dst_task, l4_cap_idx_t src_task,
            l4_fpage_t snd_fpage, l4_umword_t snd_base) L4_NOTHROW
{
  return l4_task_map_u(dst_task, src_task, snd_fpage, snd_base, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_task_unmap(l4_cap_idx_t task, l4_fpage_t fpage,
              l4_umword_t map_mask) L4_NOTHROW
{
  return l4_task_unmap_u(task, fpage, map_mask, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_task_unmap_batch(l4_cap_idx_t task, l4_fpage_t const *fpages,
                    unsigned num_fpages, l4_umword_t map_mask) L4_NOTHROW
{
  return l4_task_unmap_batch_u(task, fpages, num_fpages, map_mask,
                               l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_task_delete_obj_u(l4_cap_idx_t task, l4_cap_idx_t obj,
                     l4_utcb_t *u) L4_NOTHROW
{
  return l4_task_unmap_u(task, l4_obj_fpage(obj, 0, L4_CAP_FPAGE_RWSD),
                         L4_FP_DELETE_OBJ, u);
}

L4_INLINE l4_msgtag_t
l4_task_delete_obj(l4_cap_idx_t task, l4_cap_idx_t obj) L4_NOTHROW
{
  return l4_task_delete_obj_u(task, obj, l4_utcb());
}


L4_INLINE l4_msgtag_t
l4_task_release_cap_u(l4_cap_idx_t task, l4_cap_idx_t cap,
                      l4_utcb_t *u) L4_NOTHROW
{
  return l4_task_unmap_u(task, l4_obj_fpage(cap, 0, L4_CAP_FPAGE_RWSD),
                         L4_FP_ALL_SPACES, u);
}

L4_INLINE l4_msgtag_t
l4_task_release_cap(l4_cap_idx_t task, l4_cap_idx_t cap) L4_NOTHROW
{
  return l4_task_release_cap_u(task, cap, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_task_cap_valid(l4_cap_idx_t task, l4_cap_idx_t cap) L4_NOTHROW
{
  return l4_task_cap_valid_u(task, cap, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_task_cap_equal(l4_cap_idx_t task, l4_cap_idx_t cap_a,
                  l4_cap_idx_t cap_b) L4_NOTHROW
{
  return l4_task_cap_equal_u(task, cap_a, cap_b, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_task_add_ku_mem(l4_cap_idx_t task, l4_fpage_t *ku_mem) L4_NOTHROW
{
  return l4_task_add_ku_mem_u(task, ku_mem, l4_utcb());
}
