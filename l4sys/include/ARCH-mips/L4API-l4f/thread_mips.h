#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>

/**
 * Set the user-local register (ULR) for a given `thread`.
 *
 * \param thread  The thread for which the ULR shall be set.
 * \param ulr     The value used for the ULR.
 *
 * \note The new ULR value for the thread might become active during the next
 * thread switch if the thread is running on a different CPU than the caller.
 */
L4_INLINE l4_msgtag_t
l4_thread_mips_set_ulr(l4_cap_idx_t thread, l4_umword_t ulr) L4_NOTHROW;


/**
 * Save in-flight VM state for the given vCPU thread (MIPS).
 *
 * \param thread         The vCPU thread for which the state shall be saved.
 * \param vm_state_bits  Each set bit requests that the corresponding register
 *                       state shall be saved to the l4_vm_state_t. The bits
 *                       are defined in `L4_vm_state_modified_bits`.
 *
 * After a successfull operation the `clean_cp0_map` indicates which state is
 * now in sync with the VM. Note, this might be more than the requested state.
 */
L4_INLINE l4_msgtag_t
l4_thread_mips_save_vm_state(l4_cap_idx_t thread,
                             l4_umword_t vm_state_bits) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_thread_mips_set_ulr_u(l4_cap_idx_t thread, l4_umword_t ulr,
                         l4_utcb_t *utcb) L4_NOTHROW;


L4_INLINE l4_msgtag_t
l4_thread_mips_set_ulr_u(l4_cap_idx_t thread, l4_umword_t ulr,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = 0x10;
  l4_utcb_mr_u(utcb)->mr[1] = ulr;
  return l4_ipc_call(thread, utcb,
                     l4_msgtag(L4_PROTO_THREAD, 2, 0, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_mips_set_ulr(l4_cap_idx_t thread, l4_umword_t ulr) L4_NOTHROW
{ return l4_thread_mips_set_ulr_u(thread, ulr, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_thread_mips_save_vm_state_u(l4_cap_idx_t thread, l4_umword_t vm_state_bits,
                               l4_utcb_t *utcb) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_thread_mips_save_vm_state_u(l4_cap_idx_t thread, l4_umword_t vm_state_bits,
                               l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = 0x14;
  l4_utcb_mr_u(utcb)->mr[1] = vm_state_bits;
  return l4_ipc_call(thread, utcb,
                     l4_msgtag(L4_PROTO_THREAD, 2, 0, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_mips_save_vm_state(l4_cap_idx_t thread,
                             l4_umword_t vm_state_bits) L4_NOTHROW
{
  return l4_thread_mips_save_vm_state_u(thread, vm_state_bits, l4_utcb());
}

