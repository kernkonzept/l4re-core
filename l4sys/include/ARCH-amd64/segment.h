/**
 * \file
 * \brief   Segment handling (AMD64).
 * \ingroup api_calls_fiasco
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*****************************************************************************/
#ifndef __L4_SYS__ARCH_X86__SEGMENT_H__
#define __L4_SYS__ARCH_X86__SEGMENT_H__

#ifndef L4API_l4f
#error This header file can only be used with a L4API version!
#endif

#include <l4/sys/ipc.h>

/**
 * Set LDT segments descriptors.
 * \ingroup api_calls_fiasco
 *
 * \param	task			Task to set the segment for.
 * \param	ldt			Pointer to LDT hardware descriptors.
 * \param	num_desc                Number of descriptors.
 * \param	entry_number_start	Entry number to start.
 * \utcb{utcb}
 *
 * \retval -L4_ENOSYS The kernel configuration doesn't support this feature.
 * \retval -L4_EINVAL Invalid descriptor or invalid entry number.
 * \retval L4_EOK Success.
 *
 * \note This feature is not available if the kernel is configured with page
 *       table isolation.
 */
L4_INLINE long
fiasco_ldt_set(l4_cap_idx_t task, void *ldt, unsigned int num_desc,
               unsigned int entry_number_start, l4_utcb_t *utcb);

/**
 * Set GDT segment descriptors. Fiasco supports 4 consecutive entries, starting
 * at the value returned by fiasco_gdt_get_entry_offset().
 * \ingroup api_calls_fiasco
 *
 * \param thread		Thread to set the GDT entry for.
 * \param desc			Pointer to GDT descriptors.
 * \param size			Size of the descriptors in bytes
 *				 (multiple of 8).
 * \param entry_number_start	Entry number to start (valid values: 0-3).
 * \utcb{utcb}
 *
 * \retval <0 At least one provided GDT descriptor is considered unsafe by the
 *            kernel, and not all selected GDT descriptors have been updated.
 * \retval L4_EOK Success.
 */
L4_INLINE long
fiasco_gdt_set(l4_cap_idx_t thread, void *desc, unsigned int size,
               unsigned int entry_number_start, l4_utcb_t *utcb);

/**
 * Return the offset of the entry in the GDT.
 * \param  thread    Thread to get info from.
 * \utcb{utcb}
 * \ingroup api_calls_fiasco
 */
L4_INLINE unsigned
fiasco_gdt_get_entry_offset(l4_cap_idx_t thread, l4_utcb_t *utcb);

/**
 * \brief Contants for LDT handling.
 */
enum L4_task_ldt_x86_consts
{
  /** Size of an LDT entry.  */
  L4_TASK_LDT_X86_ENTRY_SIZE = 8,
  /** Maximum number of LDT entries that can be written with one call. */
  L4_TASK_LDT_X86_MAX_ENTRIES
    = (L4_UTCB_GENERIC_DATA_SIZE - 2)
      / (L4_TASK_LDT_X86_ENTRY_SIZE / (L4_MWORD_BITS / 8)),
};

/**
 * Set the base address for the FS segment.
 * \param  thread    Thread for which the FS base address shall be modified.
 * \param  base      Base address.
 * \utcb{utcb}
 *
 * \retval L4_EOK      Success.
 * \retval -L4_EINVAL  Invalid base address (\p base).
 * \retval -L4_ENOSYS  Operation not supported with current kernel
 *                     configuration.
 *
 * \note
 *   Calling this function is equivalent to calling
 *   `fiasco_amd64_set_segment_base(thread, L4_AMD64_SEGMENT_FS, base, utcb)`.
 */
L4_INLINE long
fiasco_amd64_set_fs(l4_cap_idx_t thread, l4_umword_t base, l4_utcb_t *utcb);

/**
 * Constants for identifying segments.
 */
enum L4_sys_segment
{
  /// Constant identifying the FS segment.
  L4_AMD64_SEGMENT_FS = 0,
  /// Constant identifying the GS segment.
  L4_AMD64_SEGMENT_GS = 1
};

/**
 * Set the base address for a segment.
 * \param  thread    Thread for which the base address of the selected segment
 *                   shall be modified.
 * \param  segr      Segment to modify (one of ::L4_sys_segment).
 * \param  base      Base address.
 * \utcb{utcb}
 *
 * \retval L4_EOK      Success.
 * \retval -L4_EINVAL  Invalid segment (\p segr) or base address (\p base).
 * \retval -L4_ENOSYS  Operation not supported with current kernel
 *                     configuration.
 */
L4_INLINE long
fiasco_amd64_set_segment_base(l4_cap_idx_t thread, enum L4_sys_segment segr,
                              l4_umword_t base, l4_utcb_t *utcb);

/**
 * Get segment information.
 * \param[in]  thread     Thread to get info from.
 * \param[out] user_ds    DS segment selector.
 * \param[out] user_cs    64-bit CS segment selector.
 * \param[out] user32_cs  32-bit CS segment selector.
 * \utcb{utcb}
 * \return System call error
 */
L4_INLINE long
fiasco_amd64_segment_info(l4_cap_idx_t thread, unsigned *user_ds,
                          unsigned *user_cs, unsigned *user32_cs,
                          l4_utcb_t *utcb);

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

#include <l4/sys/task.h>
#include <l4/sys/thread.h>

L4_INLINE long
fiasco_ldt_set(l4_cap_idx_t task, void *ldt, unsigned int num_desc,
               unsigned int entry_number_start, l4_utcb_t *utcb)
{
  if (num_desc > L4_TASK_LDT_X86_MAX_ENTRIES)
    return -L4_EINVAL;
  l4_utcb_mr_u(utcb)->mr[0] = L4_TASK_LDT_SET_X86_OP;
  l4_utcb_mr_u(utcb)->mr[1] = entry_number_start;
  __builtin_memcpy(&l4_utcb_mr_u(utcb)->mr[2], ldt,
                   num_desc * L4_TASK_LDT_X86_ENTRY_SIZE);
  return l4_error_u(l4_ipc_call(task, utcb, l4_msgtag(L4_PROTO_TASK, 2 + num_desc * 2, 0, 0), L4_IPC_NEVER), utcb);
}

L4_INLINE unsigned
fiasco_gdt_get_entry_offset(l4_cap_idx_t thread, l4_utcb_t *utcb)
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_THREAD_X86_GDT_OP;
  if (l4_error_u(l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 1, 0, 0), L4_IPC_NEVER), utcb))
    return -1;
  return l4_utcb_mr_u(utcb)->mr[0];
}

L4_INLINE long
fiasco_amd64_segment_info(l4_cap_idx_t thread, unsigned *user_ds,
                          unsigned *user_cs, unsigned *user32_cs,
                          l4_utcb_t *utcb)
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  int r;

  m->mr[0] = L4_THREAD_AMD64_GET_SEGMENT_INFO_OP;

  r = l4_error_u(l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 1, 0, 0),
                             L4_IPC_NEVER), utcb);
  if (r < 0)
    return r;

  *user_ds   = m->mr[0];
  *user_cs   = m->mr[1];
  *user32_cs = m->mr[2];

  return 0;
}

#endif /* ! __L4_SYS__ARCH_X86__SEGMENT_H__ */
