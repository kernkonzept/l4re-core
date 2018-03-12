/*****************************************************************************/
/**
 * \file
 * Common L4 ABI Data Types.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2013 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
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
/*****************************************************************************/
#pragma once

#include <l4/sys/l4int.h>
#include <l4/sys/compiler.h>
#include <l4/sys/consts.h>


/**
 * \defgroup l4_msgtag_api Message Tag
 * \ingroup l4_ipc_api
 * API related to the message tag data type.
 *
 * \includefile{l4/sys/types.h}
 */

/**
 * Message tag for IPC operations.
 * \ingroup l4_msgtag_api
 *
 * All predefined protocols used by the kernel.
 */
enum l4_msgtag_protocol
{
  L4_PROTO_NONE          = 0,   ///< Default protocol tag to reply to kernel
  L4_PROTO_ALLOW_SYSCALL = 1,   ///< Allow an alien the system call
  L4_PROTO_PF_EXCEPTION  = 1,   ///< Make an exception out of a page fault

  L4_PROTO_IRQ           =  -1L, ///< IRQ message
  L4_PROTO_PAGE_FAULT    =  -2L, ///< Page fault message
  L4_PROTO_PREEMPTION    =  -3L, ///< Preemption message
  L4_PROTO_SYS_EXCEPTION =  -4L, ///< System exception
  L4_PROTO_EXCEPTION     =  -5L, ///< Exception
  L4_PROTO_SIGMA0        =  -6L, ///< Sigma0 protocol
  L4_PROTO_IO_PAGE_FAULT =  -8L, ///< I/O page fault message
  L4_PROTO_KOBJECT       = -10L, ///< Protocol for messages to a a generic kobject
  L4_PROTO_TASK          = -11L, ///< Protocol for messages to a task object
  L4_PROTO_THREAD        = -12L, ///< Protocol for messages to a thread object
  L4_PROTO_LOG           = -13L, ///< Protocol for messages to a log object
  L4_PROTO_SCHEDULER     = -14L, ///< Protocol for messages to a scheduler object
  L4_PROTO_FACTORY       = -15L, ///< Protocol for messages to a factory object
  L4_PROTO_VM            = -16L, ///< Protocol for messages to a virtual machine object
  L4_PROTO_DMA_SPACE     = -17L, ///< Protocol for (creating) kernel DMA space objects
  L4_PROTO_IRQ_SENDER    = -18L, ///< Protocol for IRQ senders (IRQ -> IPC)
  L4_PROTO_IRQ_MUX       = -19L, ///< Protocol for IRQ mux (IRQ -> n x IRQ)
  L4_PROTO_SEMAPHORE     = -20L, ///< Protocol for semaphore objects
  L4_PROTO_META          = -21L, ///< Meta information protocol
  L4_PROTO_IOMMU         = -22L, ///< Protocol ID for IO-MMUs
  L4_PROTO_DEBUGGER      = -23L, ///< Protocol ID for the ddebugger
};

enum L4_varg_type
{
  L4_VARG_TYPE_NIL    = 0x00,
  L4_VARG_TYPE_UMWORD = 0x01,
  L4_VARG_TYPE_MWORD  = 0x81,
  L4_VARG_TYPE_STRING = 0x02,
  L4_VARG_TYPE_FPAGE  = 0x03,

  L4_VARG_TYPE_SIGN   = 0x80,
};


/**
 * Flags for message tags.
 * \ingroup l4_msgtag_api
 */
enum l4_msgtag_flags
{
  // flags for received IPC
  /**
   * Error indicator flag.
   * \hideinitializer
   */
  L4_MSGTAG_ERROR        = 0x8000,
  /**
   * Cross-CPU invocation indicator flag.
   * \hideinitializer
   */
  L4_MSGTAG_XCPU         = 0x4000,

  // flags for sending IPC
  /**
   * Enable FPU transfer flag for IPC.
   * \hideinitializer
   *
   * By enabling this flag when sending IPC, the sender indicates that the
   * contents of the FPU shall be transfered to the receiving thread.
   * However, the receiver has to indicate its willingness to receive
   * FPU context in its buffer descriptor register (BDR).
   */
  L4_MSGTAG_TRANSFER_FPU = 0x1000,
  /**
   * Enable schedule in IPC flag.
   * \hideinitializer
   *
   * Usually IPC operations donate the remaining time slice of a thread
   * to the called thread. Enabling this flag when sending IPC does a real
   * scheduling decision. However, this flag decreases IPC performance.
   */
  L4_MSGTAG_SCHEDULE     = 0x2000,
  /**
   * Enable IPC propagation.
   * \hideinitializer
   *
   * This flag enables IPC propagation, which means an IPC reply-connection
   * from the current caller will be propagated to the new IPC receiver.
   * This makes it possible to propagate an IPC call to a third thread, which
   * may then directly answer to the caller.
   */
  L4_MSGTAG_PROPAGATE    = 0x4000,

  /**
   * Mask for all flags.
   * \hideinitializer
   */
  L4_MSGTAG_FLAGS        = 0xf000,
};


/**
 * Message tag data structure.
 * \ingroup l4_msgtag_api
 *
 * \includefile{l4/sys/types.h}
 *
 * Describes the details of an IPC operation, in particular
 * which parts of the UTCB have to be transmitted, and also flags
 * to enable real-time and FPU extensions.
 *
 * The message tag also contains a user-defined label that could be used
 * to specify a protocol ID. Some negative values are reserved for kernel
 * protocols such as page faults and exceptions.
 *
 * The type must be treated completely opaque.
 */
typedef struct l4_msgtag_t
{
  l4_mword_t raw;   ///< raw value
#ifdef __cplusplus
  /// Get the protocol value.
  long label() const throw() { return raw >> 16; }
  /// Set the protocol value.
  void label(long v) throw() { raw = (raw & 0x0ffff) | (v << 16); }
  /// Get the number of untyped words.
  unsigned words() const throw() { return raw & 0x3f; }
  /// Get the number of typed items.
  unsigned items() const throw() { return (raw >> 6) & 0x3f; }
  /**
   * Get the flags value.
   *
   * The flags are a combination of the flags defined by
   * #l4_msgtag_flags.
   */
  unsigned flags() const throw() { return raw & 0xf000; }
  /// Test if protocol indicates page-fault protocol.
  bool is_page_fault() const throw() { return label() == L4_PROTO_PAGE_FAULT; }
  /// Test if protocol indicates preemption protocol.
  bool is_preemption() const throw() { return label() == L4_PROTO_PREEMPTION; }
  /// Test if protocol indicates system-exception protocol.
  bool is_sys_exception() const throw() { return label() == L4_PROTO_SYS_EXCEPTION; }
  /// Test if protocol indicates exception protocol.
  bool is_exception() const throw() { return label() == L4_PROTO_EXCEPTION; }
  /// Test if protocol indicates sigma0 protocol.
  bool is_sigma0() const throw() { return label() == L4_PROTO_SIGMA0; }
  /// Test if protocol indicates IO-page-fault protocol.
  bool is_io_page_fault() const throw() { return label() == L4_PROTO_IO_PAGE_FAULT; }
  /// Test if flags indicate an error.
  unsigned has_error() const throw() { return raw & L4_MSGTAG_ERROR; }
#endif
} l4_msgtag_t;



/**
 * Create a message tag from the specified values.
 * \ingroup l4_msgtag_api
 *
 * \param label  The user-defined label
 * \param words  The number of untyped words within the UTCB
 * \param items  The number of typed items (e.g., flex pages) within the UTCB
 * \param flags  The IPC flags for realtime and FPU extensions
 *
 * \return Message tag
 */
L4_INLINE l4_msgtag_t l4_msgtag(long label, unsigned words, unsigned items,
                                unsigned flags) L4_NOTHROW;

/**
 * Get the protocol of tag.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Label
 */
L4_INLINE long l4_msgtag_label(l4_msgtag_t t) L4_NOTHROW;

/**
 * Get the number of untyped words.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Number of words
 */
L4_INLINE unsigned l4_msgtag_words(l4_msgtag_t t) L4_NOTHROW;

/**
 * Get the number of typed items.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Number of items.
 */
L4_INLINE unsigned l4_msgtag_items(l4_msgtag_t t) L4_NOTHROW;

/**
 * Get the flags.
 * \ingroup l4_msgtag_api
 *
 * The flag are defined by #l4_msgtag_flags.
 *
 * \param t  The tag
 *
 * \return Flags
 */
L4_INLINE unsigned l4_msgtag_flags(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for error indicator flag.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return >0 for yes, 0 for no
 *
 * Return whether the kernel operation caused a communication error, e.g.
 * with IPC.
 * if true: utcb->error is valid, otherwise utcb->error is not valid
 */
L4_INLINE unsigned l4_msgtag_has_error(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for page-fault protocol.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Boolean value
 */
L4_INLINE unsigned l4_msgtag_is_page_fault(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for preemption protocol.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 * \return Boolean value
 */
L4_INLINE unsigned l4_msgtag_is_preemption(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for system-exception protocol.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Boolean value
 */
L4_INLINE unsigned l4_msgtag_is_sys_exception(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for exception protocol.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Boolean value
 */
L4_INLINE unsigned l4_msgtag_is_exception(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for sigma0 protocol.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Boolean value
 */
L4_INLINE unsigned l4_msgtag_is_sigma0(l4_msgtag_t t) L4_NOTHROW;

/**
 * Test for IO-page-fault protocol.
 * \ingroup l4_msgtag_api
 *
 * \param t  The tag
 *
 * \return Boolean value
 */
L4_INLINE unsigned l4_msgtag_is_io_page_fault(l4_msgtag_t t) L4_NOTHROW;

/**
 * \defgroup l4_cap_api Capabilities
 * \ingroup l4_api
 * C interface for capabilities.
 *
 * Add
 *
 *     #include <l4/sys/types.h>
 *     #include <l4/sys/consts.h>
 *
 * to your code to use the functions and definitions explained here.
 */
/**
 * L4 Capability selector Type.
 * \ingroup l4_cap_api
 */
typedef unsigned long l4_cap_idx_t;

/**
 * Test if a capability selector is the invalid capability.
 * \ingroup l4_cap_api
 *
 * \param c  Capability selector
 *
 * \retval 0   The capability selector is not the invalid capability.
 * \retval >0  The capability selector is the invalid capability.
 */
L4_INLINE unsigned l4_is_invalid_cap(l4_cap_idx_t c) L4_NOTHROW;

/**
 * Test if a capability selector is a valid selector.
 * \ingroup l4_cap_api
 *
 * \param c  Capability selector
 *
 * \retval 0   The capability selector is not valid.
 * \retval >0  The capability selector is valid.
 */
L4_INLINE unsigned l4_is_valid_cap(l4_cap_idx_t c) L4_NOTHROW;

/**
 * Test if two capability selectors are equal.
 * \ingroup l4_cap_api
 *
 * \param c1  Capability
 * \param c2  Capability
 *
 * \retval 0  The given capabilities are not in the same slot.
 * \retval 1  The given capabilities are in the same slot.
 */
L4_INLINE unsigned l4_capability_equal(l4_cap_idx_t c1, l4_cap_idx_t c2) L4_NOTHROW;

/**
 * Get the next capability selector after `c`.
 *
 * \param c  The capability selector for which the next selector shall be
 *           computed.
 *
 * \returns The next capability selector after `c`.
 */
L4_INLINE l4_cap_idx_t l4_capability_next(l4_cap_idx_t c) L4_NOTHROW;

/* ************************************************************************* */
/* Implementation */

L4_INLINE unsigned
l4_is_invalid_cap(l4_cap_idx_t c) L4_NOTHROW
{ return c & L4_INVALID_CAP_BIT; }

L4_INLINE unsigned
l4_is_valid_cap(l4_cap_idx_t c) L4_NOTHROW
{ return !(c & L4_INVALID_CAP_BIT); }

L4_INLINE unsigned
l4_capability_equal(l4_cap_idx_t c1, l4_cap_idx_t c2) L4_NOTHROW
{ return (c1 >> L4_CAP_SHIFT) == (c2 >> L4_CAP_SHIFT); }


/**
 * Message tag functions
 */
L4_INLINE
l4_msgtag_t l4_msgtag(long label, unsigned words, unsigned items,
                      unsigned flags) L4_NOTHROW
{
  return (l4_msgtag_t){(label << 16) | (l4_mword_t)(words & 0x3f)
                       | (l4_mword_t)((items & 0x3f) << 6)
                       | (l4_mword_t)(flags & 0xf000)};
}



L4_INLINE
long l4_msgtag_label(l4_msgtag_t t) L4_NOTHROW
{ return t.raw >> 16; }

L4_INLINE
unsigned l4_msgtag_words(l4_msgtag_t t) L4_NOTHROW
{ return t.raw & 0x3f; }

L4_INLINE
unsigned l4_msgtag_items(l4_msgtag_t t) L4_NOTHROW
{ return (t.raw >> 6) & 0x3f; }

L4_INLINE
unsigned l4_msgtag_flags(l4_msgtag_t t) L4_NOTHROW
{ return t.raw & 0xf000; }


L4_INLINE
unsigned l4_msgtag_has_error(l4_msgtag_t t) L4_NOTHROW
{ return t.raw & L4_MSGTAG_ERROR; }



L4_INLINE unsigned l4_msgtag_is_page_fault(l4_msgtag_t t) L4_NOTHROW
{ return l4_msgtag_label(t) == L4_PROTO_PAGE_FAULT; }

L4_INLINE unsigned l4_msgtag_is_preemption(l4_msgtag_t t) L4_NOTHROW
{ return l4_msgtag_label(t) == L4_PROTO_PREEMPTION; }

L4_INLINE unsigned l4_msgtag_is_sys_exception(l4_msgtag_t t) L4_NOTHROW
{ return l4_msgtag_label(t) == L4_PROTO_SYS_EXCEPTION; }

L4_INLINE unsigned l4_msgtag_is_exception(l4_msgtag_t t) L4_NOTHROW
{ return l4_msgtag_label(t) == L4_PROTO_EXCEPTION; }

L4_INLINE unsigned l4_msgtag_is_sigma0(l4_msgtag_t t) L4_NOTHROW
{ return l4_msgtag_label(t) == L4_PROTO_SIGMA0; }

L4_INLINE unsigned l4_msgtag_is_io_page_fault(l4_msgtag_t t) L4_NOTHROW
{ return l4_msgtag_label(t) == L4_PROTO_IO_PAGE_FAULT; }

L4_INLINE l4_cap_idx_t l4_capability_next(l4_cap_idx_t c) L4_NOTHROW
{ return c + L4_CAP_OFFSET; }

#include <l4/sys/__l4_fpage.h>
#include <l4/sys/__timeout.h>
