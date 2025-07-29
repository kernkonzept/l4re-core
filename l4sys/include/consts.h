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
 * License: see LICENSE.spdx (in this directory or the directories above)
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
 *
 * The following combinations of flags are supported when invoking IPC (see
 * l4_ipc()); with other combinations, behavior is undefined:
 *
 * - #L4_SYSF_SEND:
 *   send to specified partner
 * - #L4_SYSF_RECV:
 *   receive from specified partner
 * - #L4_SYSF_RECV | #L4_SYSF_OPEN_WAIT:
 *   receive from any sending partner; see #L4_SYSF_WAIT
 * - #L4_SYSF_SEND | #L4_SYSF_RECV:
 *   call specified partner; see #L4_SYSF_CALL
 * - #L4_SYSF_SEND | #L4_SYSF_RECV | #L4_SYSF_OPEN_WAIT:
 *   send to specified partner and receive from any sending partner;
 *   see #L4_SYSF_SEND_AND_WAIT
 * - #L4_SYSF_REPLY | #L4_SYSF_SEND:
 *   reply to caller
 * - #L4_SYSF_REPLY | #L4_SYSF_SEND | #L4_SYSF_RECV:
 *   call the caller
 * - #L4_SYSF_REPLY | #L4_SYSF_SEND | #L4_SYSF_RECV | #L4_SYSF_OPEN_WAIT:
 *   reply to caller and receive from any sending partner;
 *   see #L4_SYSF_REPLY_AND_WAIT
 */
enum l4_syscall_flags_t
{
  /**
   * Empty set of flags.
   * \hideinitializer
   */
  L4_SYSF_NONE      = 0x00,

  /**
   * Send-phase flag.
   * \hideinitializer
   *
   * Setting this flag in a capability selector induces a send phase,
   * this means a message is sent to the object denoted by the capability.
   * For receive phase see #L4_SYSF_RECV.
   *
   * In l4_vcpu_state_t::user_task this flag means that the kernel has cached
   * the user task capability internally, see l4_thread_vcpu_resume_commit().
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
   * Combines #L4_SYSF_SEND and #L4_SYSF_RECV.
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
  /** \deprecated Superseded by #L4_CAP_OFFSET. */
  L4_CAP_SIZE    = 1UL << L4_CAP_SHIFT,
  /** Offset of two consecutive capability selectors. \hideinitializer */
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
  L4_SCHED_MIN_PRIO = 1,
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
   * Flag to tell the unmap operation to revoke permissions from all child
   * mappings including the mapping in the invoked task.
   *
   * \note Object capabilities are not hierarchical -- they have no children.
   *       The result of the map operation on an object capability is a copy of
   *       that capability in the object space of the destination task. An
   *       unmap operation on object capabilities is a no-op if this flag is
   *       not specified.
   * \hideinitializer
   * \see L4::Task::unmap() l4_task_unmap()
   */
  L4_FP_ALL_SPACES   = 0x80000000UL,

  /**
   * Flag that indicates that an unmap operation on object capabilities shall
   * try to delete the corresponding objects immediately. This flag implies the
   * #L4_FP_ALL_SPACES flag. The concept of deletion is only applicable to
   * kernel objects. Therefore, for memory and I/O port capabilities, this flag
   * has the same effect as #L4_FP_ALL_SPACES alone.
   * \hideinitializer
   * \see L4::Task::unmap() l4_task_unmap()
   *
   * \note Specifying #L4_FP_DELETE_OBJ ^ #L4_FP_ALL_SPACES is treated as
   *       #L4_FP_OTHER_SPACES.
   */
  L4_FP_DELETE_OBJ   = 0xc0000000UL,

  /**
   * Counterpart to #L4_FP_ALL_SPACES; revoke permissions from child mappings
   * only.
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
  /**
   * Flag as \em grant instead of \em map operation. This means, the sender
   * delegates access to the receiver and the kernel removes the rights from the
   * sender (basically a move operation). The mapping in the receiver gets the
   * new parent of any child mappings of the mapping of the sender. Rights
   * revocation via send item/flexpage is *not* guaranteed to be applied to
   * descendant mappings in case of grant. See \ref l4re_concepts_mapping for
   * more details on map/grant.
   *
   * \note The grant operation is not performed if the resulting rights of the
   *       receiver mapping would not contain the #L4_CAP_FPAGE_R bit (for
   *       object capabilities) or none of the #L4_FPAGE_RWX bits (memory and
   *       IO ports). In that case, the mapping is not created in the receiver
   *       space and not removed from the sender space.
   *
   * \note If the removal of the whole mapping from the sender is not possible
   *       because the size of the mapped frame at the sender exceeds the size
   *       defined by the send or receive flexpage, the grant operation is
   *       turned into a regular map operation and the mapping is _not_ removed
   *       from the sender. This would happen if, for example, a smaller part
   *       of an L4 superpage mapping shall be granted.
   */
  L4_MAP_ITEM_GRANT = 2,

  L4_MAP_ITEM_MAP   = 0, ///< Flag as usual \em map operation.

  // receive
  /**
   * This flag specifies if received capabilities shall be mapped to a
   * particular task instead of the invoking task.
   *
   * This flag may be used only if #L4_RCV_ITEM_LOCAL_ID is unset.
   *
   * Setting this flag increases the size of the buffer item by one word. This
   * word is used to specify a capability index for the task that shall receive
   * the mappings.
   */
  L4_RCV_ITEM_FORWARD_MAPPINGS = 1,

  /**
   * Mark the receive buffer to be a small receive item that describes a buffer
   * for a single object capability.
   *
   * A receive item needs to specify a *receive window*. The receive window
   * determines which kind of capabilities (object, memory, I/O ports) may be
   * received where in the respective space. If this flag is unset, the receive
   * window is specified in the second word of the receive item via a
   * [flexpage](#l4_fpage_api). If this flag is set, the receive window consists
   * of a single capability index in the object space and the capability index
   * is specified in the most significant bits of the first word of the receive
   * item (see #L4_CAP_SHIFT).
   */
  L4_RCV_ITEM_SINGLE_CAP = L4_ITEM_MAP | 2,

  /**
   * The receiver requests to receive a local ID instead of a mapping whenever
   * possible.
   *
   * This flag may be used only if #L4_RCV_ITEM_SINGLE_CAP is set and
   * #L4_RCV_ITEM_FORWARD_MAPPINGS is unset.
   *
   * When this flag is set, then,
   *
   * - when sender and receiver are bound to the same task, then no mapping is
   *   done for this item and just the raw flexpage (#l4_fpage_t) is
   *   transferred,
   * - otherwise, when the sender specified an IPC gate for transfer that is
   *   bound to a thread that is bound to the same task as the receiving thread,
   *   then no mapping is done for this item and just the bitwise OR (`|`) of
   *   the label and the #L4_CAP_FPAGE_W and #L4_CAP_FPAGE_S permissions that
   *   would have been mapped is transferred,
   * - otherwise a regular mapping is done for this item.
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
 * \attention These constants do not have any particular meaning for
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
  /**
   * Capability selector for the pager gate.
   *
   * \hideinitializer
   * For Sigma0, the pager is not present since it never raises page faults.
   * For Moe, the pager is set to Sigma0.
   */
  L4_BASE_PAGER_CAP     = 4UL << L4_CAP_SHIFT,
  /**
   * Capability selector for the log object.
   *
   * \hideinitializer
   * Present if the corresponding feature is turned on in the microkernel
   * configuration.
   */
  L4_BASE_LOG_CAP       = 5UL << L4_CAP_SHIFT,
  /// Capability selector for the base icu object.   \hideinitializer
  L4_BASE_ICU_CAP       = 6UL << L4_CAP_SHIFT,
  /// Capability selector for the scheduler cap.   \hideinitializer
  L4_BASE_SCHEDULER_CAP = 7UL << L4_CAP_SHIFT,
  /**
   * Capability selector for the IO-MMU cap.
   *
   * \hideinitializer
   * Present if the microkernel detected an IO-MMU.
   */
  L4_BASE_IOMMU_CAP     = 8UL << L4_CAP_SHIFT,
  /**
   * Capability selector for the debugger cap.
   *
   * \hideinitializer
   * Present if the corresponding feature is turned on in the microkernel
   * configuration.
   */
  L4_BASE_DEBUGGER_CAP  = 10UL << L4_CAP_SHIFT,
  /** Capability selector for the ARM SMCCC cap.
   *
   * \hideinitializer
   * Present if the microkernel detected an ARM SMC capable trusted execution
   * environment.
   */
  L4_BASE_ARM_SMCCC_CAP = 11UL << L4_CAP_SHIFT,

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
 * The address is rounded down to the next lower minimal page boundary. On
 * most architectures this is a 4k page. Check #L4_PAGESIZE for the minimal
 * page size.
 *
 * \param address  The address to round.
 */
L4_INLINE l4_addr_t l4_trunc_page(l4_addr_t address) L4_NOTHROW;
L4_INLINE l4_addr_t l4_trunc_page(l4_addr_t address) L4_NOTHROW
{ return address & L4_PAGEMASK; }

/**
 * Round an address down to the next lower flexpage with size \a bits.
 * \ingroup l4_memory_api
 *
 * \param address  The address to round.
 * \param bits     The size of the flexpage (log2).
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
L4_INLINE l4_addr_t l4_round_size(l4_addr_t value, unsigned char bits) L4_NOTHROW;
L4_INLINE l4_addr_t l4_round_size(l4_addr_t value, unsigned char bits) L4_NOTHROW
{ return (value + (1UL << bits) - 1) & (~0UL << bits); }

/**
 * Determine how many machine words (l4_umword_t) are required to store a
 * buffer of 'size' bytes.
 *
 * \ingroup l4_memory_api
 *
 * \param size    The number of bytes to be translated into machine words.
 */
L4_INLINE unsigned l4_bytes_to_mwords(unsigned size) L4_NOTHROW;
L4_INLINE unsigned l4_bytes_to_mwords(unsigned size) L4_NOTHROW
{ return (size + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t); }

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
#elif __cplusplus >= 201103L
# define NULL nullptr
#else
# define NULL 0
#endif
#endif

#endif /* ! __L4_SYS__INCLUDE__CONSTS_H__ */
