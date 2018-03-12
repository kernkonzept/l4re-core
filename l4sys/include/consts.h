/**
 * \file
 * Common constants.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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
#ifndef __L4_SYS__INCLUDE__CONSTS_H__
#define __L4_SYS__INCLUDE__CONSTS_H__

#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

/**
 * Capability selector flags.
 * \ingroup l4_ipc_api
 *
 * These flags determine the concrete operation when a kernel object
 * is invoked.
 */
enum l4_syscall_flags_t
{
  /**
   * Default flags (call to a kernel object).
   * \hideinitializer
   *
   * Using this value as flags in the capability selector for an
   * invocation indicates a call (send and wait for a reply).
   */
  L4_SYSF_NONE      = 0x00,

  /**
   * Send-phase flag.
   * \hideinitializer
   *
   * Setting this flag in a capability selector induces a send phase,
   * this means a message is send to the object denoted by the capability.
   * For receive phase see #L4_SYSF_RECV.
   */
  L4_SYSF_SEND      = 0x01,

  /**
   * Receive-phase flag.
   * \hideinitializer
   *
   * Setting this flag in a capability selector induces a receive phase,
   * this means the invoking thread waits for a message from the object
   * denoted by the capability.
   * For a send phase see #L4_SYSF_SEND.
   */
  L4_SYSF_RECV      = 0x02,

  /**
   * Open-wait flag.
   * \hideinitializer
   *
   * This flag indicates that the receive operation (see #L4_SYSF_RECV)
   * shall be an \em open \em wait. \em Open \em wait means that the invoking
   * thread shall wait for a message from any possible sender and \em not from
   * the sender denoted by the capability.
   */
  L4_SYSF_OPEN_WAIT = 0x04,

  /**
   * Reply flag.
   * \hideinitializer
   *
   * This flag indicates that the send phase shall use the in-kernel reply
   * capability instead of the capability denoted by the selector index.
   */
  L4_SYSF_REPLY     = 0x08,

  /**
   * Call flags (combines send and receive).
   * \hideinitializer
   *
   * Combines #L4_SYSF_SEND and L4_SYSF_RECV.
   */
  L4_SYSF_CALL           = L4_SYSF_SEND | L4_SYSF_RECV,

  /**
   * Wait flags (combines receive and open wait).
   * \hideinitializer
   *
   * Combines #L4_SYSF_RECV and #L4_SYSF_OPEN_WAIT.
   */
  L4_SYSF_WAIT           = L4_SYSF_OPEN_WAIT | L4_SYSF_RECV,

  /**
   * Send-and-wait flags.
   * \hideinitializer
   *
   * Combines #L4_SYSF_SEND and #L4_SYSF_WAIT.
   */
  L4_SYSF_SEND_AND_WAIT  = L4_SYSF_OPEN_WAIT | L4_SYSF_CALL,

  /**
   * Reply-and-wait flags.
   * \hideinitializer
   *
   * Combines #L4_SYSF_SEND, #L4_SYSF_REPLY, and #L4_SYSF_WAIT.
   */
  L4_SYSF_REPLY_AND_WAIT = L4_SYSF_WAIT | L4_SYSF_SEND | L4_SYSF_REPLY
};

/**
 * Constants related to capability selectors.
 * \ingroup l4_cap_api
 */
enum l4_cap_consts_t
{
  /** Capability index shift. \hideinitializer */
  L4_CAP_SHIFT   = 12UL,
  /** Offset of two consecutive capability selectors. \hideinitializer */
  L4_CAP_SIZE    = 1UL << L4_CAP_SHIFT,
  L4_CAP_OFFSET  = 1UL << L4_CAP_SHIFT,
  /**
   * Mask to get only the relevant bits of an l4_cap_idx_t.
   * \hideinitializer
   */
  L4_CAP_MASK    = ~0UL << (L4_CAP_SHIFT - 1),
  /** Invalid capability selector. \hideinitializer */
  L4_INVALID_CAP = ~0UL << (L4_CAP_SHIFT - 1),

  L4_INVALID_CAP_BIT = 1UL << (L4_CAP_SHIFT - 1),
};

enum l4_sched_consts_t
{
  L4_SCHED_MIN_PRIO = 0,
  L4_SCHED_MAX_PRIO = 255,
};

/**
 * Flags for the unmap operation.
 * \ingroup l4_task_api
 * \see L4::Task::unmap() and l4_task_unmap()
 */
enum l4_unmap_flags_t
{
  /**
   * Flag to tell the unmap operation to unmap all child mappings including the
   * mapping in the invoked task.
   * \hideinitializer
   * \see L4::Task::unmap() l4_task_unmap()
   */
  L4_FP_ALL_SPACES   = 0x80000000UL,

  /**
   * Flag that indicates that the unmap operation on a capability shall try to
   * delete the corresponding objects immediately.
   * \hideinitializer
   * \see L4::Task::unmap() l4_task_unmap()
   */
  L4_FP_DELETE_OBJ   = 0xc0000000UL,

  /**
   * Counterpart to #L4_FP_ALL_SPACES, unmap only child mappings.
   * \hideinitializer
   * \see L4::Task::unmap() l4_task_unmap()
   */
  L4_FP_OTHER_SPACES = 0x0UL
};

/**
 * Constants for message items.
 * \ingroup l4_msgitem_api
 */
enum l4_msg_item_consts_t
{
  L4_ITEM_MAP       = 8, ///< Identify a message item as \em map \em item.

  /**
   * Denote that the following item shall be put into the same receive item as
   * this one.
   */
  L4_ITEM_CONT      = 1,

  // send
  L4_MAP_ITEM_GRANT = 2, ///< Flag as \em grant instead of \em map operation.
  L4_MAP_ITEM_MAP   = 0, ///< Flag as usual \em map operation.

  // receive
  /**
   * Mark the receive buffer to be a small receive item that describes a buffer
   * for a single capability.
   */
  L4_RCV_ITEM_SINGLE_CAP = L4_ITEM_MAP | 2,

  /**
   * The receiver requests to receive a local ID instead of a mapping whenever
   * possible.
   */
  L4_RCV_ITEM_LOCAL_ID   = 4,
};

/**
 * Constants for buffer descriptors.
 * \ingroup l4_utcb_br_api
 */
enum l4_buffer_desc_consts_t
{
  L4_BDR_MEM_SHIFT   = 0,  ///< Bit offset for the memory-buffer index
  L4_BDR_IO_SHIFT    = 5,  ///< Bit offset for the IO-buffer index
  L4_BDR_OBJ_SHIFT   = 10, ///< Bit offset for the capability-buffer index
  L4_BDR_OFFSET_MASK = (1UL << 20) - 1,
};

/**
 * \ingroup l4_cap_api
 * Default capabilities setup for the initial tasks.
 *
 * These capability selectors are setup per default by the micro kernel
 * for the two initial tasks, the Root-Pager (Sigma0) and the Root-Task
 * (Moe).
 *
 * \attention This constants do not have any particular meaning for
 *            applications started by Moe, see \ref api_l4re_env for
 *            this kind of information.
 * \see \ref api_l4re_env for information useful for normal user applications.
 */
enum l4_default_caps_t
{
  /// Capability selector for the current task. \hideinitializer
  L4_BASE_TASK_CAP      = 1UL << L4_CAP_SHIFT,
  /// Capability selector for the factory.      \hideinitializer
  L4_BASE_FACTORY_CAP   = 2UL << L4_CAP_SHIFT,
  /// Capability selector for the first thread. \hideinitializer
  L4_BASE_THREAD_CAP    = 3UL << L4_CAP_SHIFT,
  /// Capability selector for the pager gate.   \hideinitializer
  L4_BASE_PAGER_CAP     = 4UL << L4_CAP_SHIFT,
  /// Capability selector for the log object.   \hideinitializer
  L4_BASE_LOG_CAP       = 5UL << L4_CAP_SHIFT,
  /// Capability selector for the base icu object.   \hideinitializer
  L4_BASE_ICU_CAP       = 6UL << L4_CAP_SHIFT,
  /// Capability selector for the scheduler cap.   \hideinitializer
  L4_BASE_SCHEDULER_CAP = 7UL << L4_CAP_SHIFT,
  /// Capability selector for the IO-MMU cap.   \hideinitializer
  L4_BASE_IOMMU_CAP     = 8UL << L4_CAP_SHIFT,
  /// Capability selector for the debugger cap. \hideinitializer
  L4_BASE_DEBUGGER_CAP  = 10UL << L4_CAP_SHIFT,

  /// \internal helper must be last before L4_BASE_CAPS_LAST
  L4_BASE_CAPS_LAST_P1,
  /// Last capability index used for base capabilities
  L4_BASE_CAPS_LAST = L4_BASE_CAPS_LAST_P1 - 1
};

/**
 * \defgroup l4_memory_api Memory related
 * Memory related constants, data types and functions.
 * \ingroup l4_api
 */
/**
 * Minimal page size (in bytes).
 * \ingroup l4_memory_api
 * \hideinitializer
 */
#define L4_PAGESIZE		(1UL << L4_PAGESHIFT)

/**
 * Mask for the page number.
 * \ingroup l4_memory_api
 * \hideinitializer
 *
 * \note The most significant bits are set.
 */
#define L4_PAGEMASK		(~(L4_PAGESIZE - 1))

/**
 * Number of bits used for page offset.
 * \ingroup l4_memory_api
 * \hideinitializer
 *
 * Size of page in log2.
 */
#define L4_LOG2_PAGESIZE	L4_PAGESHIFT

/**
 * Size of a large page.
 * \ingroup l4_memory_api
 * \hideinitializer
 *
 * A large page is a \em super \em page on IA32 or a \em section on ARM.
 */
#define L4_SUPERPAGESIZE	(1UL << L4_SUPERPAGESHIFT)

/**
 * Mask for the number of a large page.
 * \ingroup l4_memory_api
 * \hideinitializer
 *
 * \note The most significant bits are set.
 */
#define L4_SUPERPAGEMASK	(~(L4_SUPERPAGESIZE - 1))

/**
 * Number of bits used as offset for a large page.
 * \ingroup l4_memory_api
 * \hideinitializer
 * Size of large page in log2
 */
#define L4_LOG2_SUPERPAGESIZE	L4_SUPERPAGESHIFT

/**
 * Round an address down to the next lower page boundary.
 * \ingroup l4_memory_api
 *
 * The address is rounded down to the next lower mininmal page boundary. On
 * most architectures this is a 4k page. Check #L4_PAGESIZE for the minimal
 * page size.
 *
 * \param address  The address to round.
 */
L4_INLINE l4_addr_t l4_trunc_page(l4_addr_t address) L4_NOTHROW;
L4_INLINE l4_addr_t l4_trunc_page(l4_addr_t address) L4_NOTHROW
{ return address & L4_PAGEMASK; }

/**
 * Round an address down to the next lower flex page with size \a bits.
 * \ingroup l4_memory_api
 *
 * \param address  The address to round.
 * \param bits     The size of the flex page (log2).
 */
L4_INLINE l4_addr_t l4_trunc_size(l4_addr_t address, unsigned char bits) L4_NOTHROW;
L4_INLINE l4_addr_t l4_trunc_size(l4_addr_t address, unsigned char bits) L4_NOTHROW
{ return address & (~0UL << bits); }

/**
 * Round address up to the next page.
 * \ingroup l4_memory_api
 *
 * The address is rounded up to the next minimal page boundary. On most
 * architectures this is a 4k page. Check #L4_PAGESIZE for the minimal page
 * size.
 *
 * \param address  The address to round up.
 */
L4_INLINE l4_addr_t l4_round_page(l4_addr_t address) L4_NOTHROW;
L4_INLINE l4_addr_t l4_round_page(l4_addr_t address) L4_NOTHROW
{ return (address + L4_PAGESIZE - 1) & L4_PAGEMASK; }

/**
 * Round value up to the next alignment with \a bits size.
 * \ingroup l4_memory_api
 *
 * \param value   The value to round up to the next size-alignment.
 * \param bits    The size of the alignment (log2).
 */
L4_INLINE l4_addr_t l4_round_size(l4_umword_t value, unsigned char bits) L4_NOTHROW;
L4_INLINE l4_addr_t l4_round_size(l4_umword_t value, unsigned char bits) L4_NOTHROW
{ return (value + (1UL << bits) - 1) & (~0UL << bits); }

/**
 * Address related constants.
 * \ingroup l4_memory_api
 */
enum l4_addr_consts_t {
  /// Invalid address.
  L4_INVALID_ADDR = ~0UL
};

/**
 * Invalid address as pointer type.
 * \ingroup l4_memory_api
 */
#define L4_INVALID_PTR ((void *)L4_INVALID_ADDR)

#ifndef NULL
#ifndef __cplusplus
# define NULL ((void *)0)  /**< \ingroup l4sys_defines
                            **  \hideinitializer
                            ** NULL
                            **/
#else
# define NULL 0
#endif
#endif

#endif /* ! __L4_SYS__INCLUDE__CONSTS_H__ */
