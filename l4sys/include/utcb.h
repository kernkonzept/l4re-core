/*****************************************************************************/
/**
 * \file
 * UTCB definitions.
 * \ingroup l4_ipc_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
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
#ifndef _L4_SYS_UTCB_H
#define _L4_SYS_UTCB_H

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

/**
 * \defgroup l4_utcb_api Virtual Registers (UTCBs)
 * \ingroup l4_ipc_api
 *
 * L4 Virtual Registers (UTCB).
 *
 * \includefile{l4/sys/utcb.h}
 *
 * The virtual registers are part of the micro-kernel API and are located in
 * the user-level thread control block (UTCB). The UTCB is a data structure
 * defined by the micro kernel and located on kernel-provided memory.
 * Each L4 thread gets a unique UTCB assigned when it is bound to a task (see
 * \link l4_thread_control_api Thread Control \endlink,
 * l4_thread_control_bind() for more information).
 *
 * The UTCB is arranged in three blocks of virtual registers.
 * - \link l4_utcb_tcr_api Thread Control Registers (TCRs) \endlink
 * - \link l4_utcb_mr_api  Message Registers (MRs) \endlink
 * - \link l4_utcb_br_api  Buffer Registers (BRs) \endlink
 *
 * To access the contents of the virtual registers the l4_utcb_mr(),
 * l4_utcb_tcr(), and l4_utcb_br() functions must be used.
 *
 */

/**
 * Opaque type for the UTCB.
 * \ingroup l4_utcb_api
 *
 * To access the contents of the virtual registers the l4_utcb_mr(),
 * l4_utcb_tcr(), and l4_utcb_br() functions must be used.
 *
 */
typedef struct l4_utcb_t l4_utcb_t;

/**
 * \defgroup l4_utcb_mr_api Message Registers (MRs)
 * \ingroup l4_utcb_api
 */

/**
 * Encapsulation of the message-register block in the UTCB.
 * \ingroup l4_utcb_mr_api
 */
typedef union l4_msg_regs_t
{
  l4_umword_t mr[L4_UTCB_GENERIC_DATA_SIZE]; /**< Message registers */
  l4_uint64_t mr64[L4_UTCB_GENERIC_DATA_SIZE / (sizeof(l4_uint64_t)/sizeof(l4_umword_t))]; /**< Message registers 64bit alias*/
} l4_msg_regs_t;

/**
 * \defgroup l4_utcb_br_api Buffer Registers (BRs)
 * \ingroup l4_utcb_api
 */
/**
 * Encapsulation of the buffer-registers block in the UTCB.
 *
 * \ingroup l4_utcb_br_api
 */
typedef struct l4_buf_regs_t
{
  /// Buffer descriptor
  l4_umword_t bdr;

  /// Buffer registers
  l4_umword_t br[L4_UTCB_GENERIC_BUFFERS_SIZE];
} l4_buf_regs_t;

/**
 * \defgroup l4_utcb_tcr_api Thread Control Registers (TCRs)
 * \ingroup l4_utcb_api
 */
/**
 * Encapsulation of the thread-control-register block of the UTCB.
 * \ingroup l4_utcb_tcr_api
 */
typedef struct l4_thread_regs_t
{
  /// System call error codes
  l4_umword_t  error;
  /// Message transfer timeout
  l4_timeout_t xfer;
  /// User values (ignored and preserved by the kernel)
  l4_umword_t  user[3];
} l4_thread_regs_t;

__BEGIN_DECLS

/**
 * \internal
 * Get the UTCB address.
 * \attention This functions should be used by libraries.
 * \ingroup l4_utcb_api
 *
 * \returns UTCB
 *
 * This is a weak function which can be overwritten by applications.
 */
L4_CV l4_utcb_t *l4_utcb_wrap(void) L4_NOTHROW L4_PURE;

/**
 * \internal
 * Get the UTCB address.
 * \ingroup l4_utcb_api
 */
L4_INLINE l4_utcb_t *l4_utcb_direct(void) L4_NOTHROW L4_PURE;

/**
 * Get the UTCB address.
 * \ingroup l4_utcb_api
 */
L4_INLINE l4_utcb_t *l4_utcb(void) L4_NOTHROW L4_PURE;

/**
 * Get the message-register block of a UTCB.
 * \ingroup l4_utcb_api
 * \returns A pointer to the message-register block of \c u.
 */
L4_INLINE l4_msg_regs_t *l4_utcb_mr(void) L4_NOTHROW L4_PURE;

/**
 * \internal
 * \param u  The UTCB pointer to access.
 */
L4_INLINE l4_msg_regs_t *l4_utcb_mr_u(l4_utcb_t *u) L4_NOTHROW L4_PURE;

/**
 * Get the buffer-register block of a UTCB.
 * \ingroup l4_utcb_api
 *
 * \returns A pointer to the buffer-register block of \c u.
 */
L4_INLINE l4_buf_regs_t *l4_utcb_br(void) L4_NOTHROW L4_PURE;

/**
 * \internal
 * \param u  The UTCB pointer to access.
 */
L4_INLINE l4_buf_regs_t *l4_utcb_br_u(l4_utcb_t *u) L4_NOTHROW L4_PURE;

/**
 * Get the thread-control-register block of a UTCB.
 * \ingroup l4_utcb_api
 * \returns A pointer to the thread-control-register block of \c u.
 */
L4_INLINE l4_thread_regs_t *l4_utcb_tcr(void) L4_NOTHROW L4_PURE;

/**
 * \internal
 * \param u  The UTCB pointer to access.
 */
L4_INLINE l4_thread_regs_t *l4_utcb_tcr_u(l4_utcb_t *u) L4_NOTHROW L4_PURE;

/**
 * \defgroup l4_utcb_exc_api Exception registers
 * \ingroup l4_utcb_mr_api
 * Overly definition of the MRs for exception messages.
 */

/**
 * Get the message-register block of a UTCB (for an exception IPC).
 * \ingroup l4_utcb_exc_api
 *
 * \returns A pointer to the exception message in \c u.
 */
L4_INLINE l4_exc_regs_t *l4_utcb_exc(void) L4_NOTHROW L4_PURE;

/**
 * \internal
 * \param u  The UTCB pointer to access.
 */
L4_INLINE l4_exc_regs_t *l4_utcb_exc_u(l4_utcb_t *u) L4_NOTHROW L4_PURE;

/**
 * Access function to get the program counter of the exception state.
 * \ingroup l4_utcb_exc_api
 *
 * \param  u  UTCB
 * \return The program counter register out of the exception state.
 */
L4_INLINE l4_umword_t l4_utcb_exc_pc(l4_exc_regs_t const *u) L4_NOTHROW L4_PURE;

/**
 * Set the program counter register in the exception state.
 * \ingroup l4_utcb_exc_api
 *
 * \param u   UTCB
 * \param pc  The program counter to set.
 *
 */
L4_INLINE void l4_utcb_exc_pc_set(l4_exc_regs_t *u, l4_addr_t pc) L4_NOTHROW;

/**
 * Get the value out of an exception UTCB that describes the type of exception.
 * \ingroup l4_utcb_exc_api
 */
L4_INLINE unsigned long l4_utcb_exc_typeval(l4_exc_regs_t const *u) L4_NOTHROW L4_PURE;

/**
 * Check whether an exception IPC is a page fault.
 * \ingroup l4_utcb_exc_api
 *
 * \returns 0 if not, != 0 if yes
 *
 * Function to check whether an exception IPC is a page fault, also applies
 * to I/O pagefaults.
 */
L4_INLINE int l4_utcb_exc_is_pf(l4_exc_regs_t const *u) L4_NOTHROW L4_PURE;

/**
 * Function to get the L4 style page fault address out of an exception.
 * \ingroup l4_utcb_exc_api
 */
L4_INLINE l4_addr_t l4_utcb_exc_pfa(l4_exc_regs_t const *u) L4_NOTHROW L4_PURE;

/**
 * Enable or disable inheritance of FPU state to receiver.
 * \ingroup l4_utcb_br_api
 */
L4_INLINE void l4_utcb_inherit_fpu(int switch_on) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE void l4_utcb_inherit_fpu_u(l4_utcb_t *u, int switch_on) L4_NOTHROW;

/**
 * \internal
 *
 * Set an absolute timeout.
 * \ingroup l4_timeout_api
 *
 * \param  pint  Point in time in clocks
 * \param  br    The buffer register the timeout shall be placed in.
 *               (\note On 32bit architectures the timeout needs two
 *               consecutive buffers.)
 * \param  utcb  Utcb to put the absolute timeout in.
 *
 * \return timeout value
 */
L4_INLINE
l4_timeout_s l4_timeout_abs_u(l4_kernel_clock_t pint, int br,
                              l4_utcb_t *utcb) L4_NOTHROW;
/**
 * Set an absolute timeout.
 * \ingroup l4_timeout_api
 *
 * \param  pint  Point in time in clocks
 * \param  br    The buffer register the timeout shall be placed in.
 *               (\note On 32bit architectures the timeout needs two
 *               consecutive buffers.)
 *
 * \note The absolute timeout value will be placed into the buffer register
 * \a br of the current thread.
 * \return timeout value
 */
L4_INLINE
l4_timeout_s l4_timeout_abs(l4_kernel_clock_t pint, int br) L4_NOTHROW;

/**
 * Get index into 64bit message registers alias from native-sized index.
 * \ingroup l4_timeout_api
 *
 * \param idx  Index to native-sized message register
 * \return Index to 64bit message register alias
 */
L4_INLINE
unsigned l4_utcb_mr64_idx(unsigned idx) L4_NOTHROW;

/**************************************************************************
 * Implementations
 **************************************************************************/

L4_INLINE l4_msg_regs_t *l4_utcb_mr_u(l4_utcb_t *u) L4_NOTHROW
{ return (l4_msg_regs_t*)((char*)u + L4_UTCB_MSG_REGS_OFFSET); }

L4_INLINE l4_buf_regs_t *l4_utcb_br_u(l4_utcb_t *u) L4_NOTHROW
{ return (l4_buf_regs_t*)((char*)u + L4_UTCB_BUF_REGS_OFFSET); }

L4_INLINE l4_thread_regs_t *l4_utcb_tcr_u(l4_utcb_t *u) L4_NOTHROW
{ return (l4_thread_regs_t*)((char*)u + L4_UTCB_THREAD_REGS_OFFSET); }

L4_INLINE l4_exc_regs_t *l4_utcb_exc_u(l4_utcb_t *u) L4_NOTHROW
{ return (l4_exc_regs_t*)((char*)u + L4_UTCB_MSG_REGS_OFFSET); }

L4_INLINE void l4_utcb_inherit_fpu_u(l4_utcb_t *u, int switch_on) L4_NOTHROW
{
  if (switch_on)
    l4_utcb_br_u(u)->bdr |= L4_UTCB_INHERIT_FPU;
  else
    l4_utcb_br_u(u)->bdr &= ~L4_UTCB_INHERIT_FPU;
}

L4_INLINE l4_utcb_t *l4_utcb(void) L4_NOTHROW
{
#ifdef L4SYS_USE_UTCB_WRAP
  return l4_utcb_wrap();
#else
  return l4_utcb_direct();
#endif
}




L4_INLINE l4_msg_regs_t *l4_utcb_mr(void) L4_NOTHROW
{ return l4_utcb_mr_u(l4_utcb()); }

L4_INLINE l4_buf_regs_t *l4_utcb_br(void) L4_NOTHROW
{ return l4_utcb_br_u(l4_utcb()); }

L4_INLINE l4_thread_regs_t *l4_utcb_tcr(void) L4_NOTHROW
{ return l4_utcb_tcr_u(l4_utcb()); }

L4_INLINE l4_exc_regs_t *l4_utcb_exc(void) L4_NOTHROW
{ return l4_utcb_exc_u(l4_utcb()); }

L4_INLINE void l4_utcb_inherit_fpu(int switch_on) L4_NOTHROW
{ l4_utcb_inherit_fpu_u(l4_utcb(), switch_on); }

L4_INLINE
l4_timeout_s l4_timeout_abs_u(l4_kernel_clock_t val, int pos,
                              l4_utcb_t *utcb) L4_NOTHROW
{
  union T
  {
    l4_kernel_clock_t t;
    l4_umword_t m[sizeof(l4_kernel_clock_t)/sizeof(l4_umword_t)];
  };
  l4_timeout_s to;
  to.t = 0x8000 | pos;
  ((union T*)(l4_utcb_br_u(utcb)->br + pos))->t = val;
  return to;
}

L4_INLINE
l4_timeout_s l4_timeout_abs(l4_kernel_clock_t val, int pos) L4_NOTHROW
{ return l4_timeout_abs_u(val, pos, l4_utcb()); }

L4_INLINE unsigned l4_utcb_mr64_idx(unsigned idx) L4_NOTHROW
{ return idx / (sizeof(l4_uint64_t) / sizeof(l4_umword_t)); }

__END_DECLS

#endif /* ! _L4_SYS_UTCB_H */
