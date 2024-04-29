/**
 * \file
 * Common IPC interface.
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
#ifndef __L4SYS__INCLUDE__L4API_FIASCO__IPC_H__
#define __L4SYS__INCLUDE__L4API_FIASCO__IPC_H__

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>
#include <l4/sys/err.h>

/**
 * \defgroup l4_ipc_api Object Invocation
 * \ingroup l4_api
 * API for L4 object invocation.
 *
 * \includefile{l4/sys/ipc.h}
 *
 * General abstractions for L4 object invocation. The basic principle is that
 * all objects are denoted by a capability that is accessed via a capability
 * selector (see \link l4_cap_api Capabilities \endlink).
 *
 * This set of functions is common to all kinds of objects provided by the
 * L4 micro kernel. The concrete semantics of an invocation depends on the
 * object that shall be invoked.
 *
 * Objects may be invoked in various ways, the most common way is to use
 * a *call* operation (l4_ipc_call()). However, there are a lot more
 * flavours available that have a semantics depending on the object.
 *
 * \see \ref l4_kernel_object_gate_api
 * \see \ref l4re_concepts_ipc
 *
 *
 * \section ipc_timeouts Timeouts during IPC
 *
 * IPC operation between two communication partners may consist of up to two
 * phases (send phase and receive phase). For both phases, a timeout may be
 * specified (send timeout and receive timeout).
 *
 * \note When IPC communication happens across CPU cores and a timeout is
 *       specified, then the counting of the timeout only begins after the
 *       target thread has been scheduled at least once. In particular, this
 *       means that an IPC timeout, including a timeout of zero, may be delayed
 *       depending on the scheduling on the target CPU core. If a higher
 *       priority thread on the target core is executing a busy loop, that delay
 *       may even be indefinitely.
 *
 * \see \ref l4_timeout_api
 */

/*****************************************************************************
 *** IPC result checking
 *****************************************************************************/

/**
 * \defgroup l4_ipc_err_api Error Handling
 * \ingroup l4_ipc_api
 * Error handling for L4 object invocation.
 *
 * \includefile{l4/sys/ipc.h}
 */

/**
 * Error codes in the *error* TCR.
 * \ingroup l4_ipc_err_api
 *
 * The error codes are accessible via the *error* TCR, see
 * #l4_thread_regs_t.error.
 */
enum l4_ipc_tcr_error_t
{
  L4_IPC_ERROR_MASK       = 0x1F, /**< Mask for error bits. */
  L4_IPC_SND_ERR_MASK     = 0x01, /**< Send error mask. */

  L4_IPC_ENOT_EXISTENT    = 0x04, /**< Non-existing destination or source.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_RETIMEOUT        = 0x03, /**< Timeout during receive operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_SETIMEOUT        = 0x02, /**< Timeout during send operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_RECANCELED       = 0x07, /**< Receive operation canceled.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_SECANCELED       = 0x06, /**< Send operation canceled.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_REMAPFAILED      = 0x11, /**< Map flexpage failed in receive
                                   **  operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_SEMAPFAILED      = 0x10, /**< Map flexpage failed in send operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_RESNDPFTO        = 0x0b, /**< Send-pagefault timeout in receive
                                   **  operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_SESNDPFTO        = 0x0a, /**< Send-pagefault timeout in send
                                   **  operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_RERCVPFTO        = 0x0d, /**< Receive-pagefault timeout in receive
                                   **  operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_SERCVPFTO        = 0x0c, /**< Receive-pagefault timeout in send
                                   **  operation.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_REABORTED        = 0x0f, /**< Receive operation aborted.
                                   **  \ingroup l4_ipc_api
                                   **/
  L4_IPC_SEABORTED        = 0x0e, /**< Send operation aborted.
                                   **  \ingroup l4_ipc_api
                                   **/
  /**
   * Received message truncated. Usually returned when the typed items to be
   * sent by the IPC partner exceed the buffer registers of the respective
   * types.
   * \ingroup l4_ipc_api
   */
  L4_IPC_REMSGCUT         = 0x09,

  /**
   * Sent message truncated. Usually returned when the typed items to be sent
   * exceed the IPC partner’s buffer registers of the respective types.
   * \ingroup l4_ipc_api
   */
  L4_IPC_SEMSGCUT         = 0x08,
};


/**
 * Get the IPC error code for an IPC operation.
 * \ingroup l4_ipc_err_api
 *
 * \param tag   Message tag returned by the IPC operation.
 * \param utcb  UTCB that was used for the IPC operation.
 *
 * \return 0 if no error condition is set,
 *         error code otherwise (see #l4_ipc_tcr_error_t).
 */
L4_INLINE l4_umword_t
l4_ipc_error(l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW;


/**
 * Get IPC error code if any or message tag label otherwise for an IPC call.
 * \ingroup l4_ipc_err_api
 *
 * This function shall only be used if the IPC operation includes a receive
 * phase (usually a call operation), otherwise no tag label is received and the
 * return value of this function is undefined.
 *
 * \param tag  Message tag returned by the IPC call.
 *
 * \return  In case of an IPC error, a negative error code in the range of
 *          #L4_EIPC_LO to #L4_EIPC_HI (see l4_ipc_to_errno() and
 *          #l4_ipc_tcr_error_t), otherwise the tag label. By convention, the
 *          callee can signal errors via a negative tag label (negated value
 *          from #l4_error_code_t) and success via a non-negative value.
 */
L4_INLINE long
l4_error(l4_msgtag_t tag) L4_NOTHROW;

L4_INLINE long
l4_error_u(l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW;

/*****************************************************************************
 *** IPC results
 *****************************************************************************/

/**
 * Returns whether an error occurred in send phase of an invocation.
 * \ingroup l4_ipc_err_api
 *
 * \pre l4_msgtag_has_error(tag) == true
 * \param utcb   UTCB to check.
 *
 * \return Boolean value.
 */
L4_INLINE int l4_ipc_is_snd_error(l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Returns whether an error occurred in receive phase of an invocation.
 * \ingroup l4_ipc_err_api
 *
 * \pre l4_msgtag_has_error(tag) == true
 * \param utcb   UTCB to check.
 *
 * \return Boolean value.
 */
L4_INLINE int l4_ipc_is_rcv_error(l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get the error condition of the last invocation from the TCR.
 * \ingroup l4_ipc_err_api
 *
 * \pre l4_msgtag_has_error(tag) == true
 * \param utcb   UTCB to check.
 *
 * \return Error condition of type l4_ipc_tcr_error_t.
 */
L4_INLINE int l4_ipc_error_code(l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get a negative error code for the given IPC error code.
 * \param ipc_error_code  IPC error code as delivered by the kernel.
 *                        (or returned by the l4_ipc_error_code() function).
 * \return negative error code in the range of #L4_EIPC_LO to #L4_EIPC_HI.
 */
L4_INLINE long l4_ipc_to_errno(unsigned long ipc_error_code) L4_NOTHROW;


/*****************************************************************************
 *** IPC calls
 *****************************************************************************/

/**
 * Send a message to an object (do \b not wait for a reply).
 * \ingroup l4_ipc_api
 *
 * \param dest     Capability selector for the destination object. A value of
 *                 #L4_INVALID_CAP denotes the current thread and could be used
 *                 for sleeping without busy waiting for the time specified in
 *                 the \c snd part of the \c timeout parameter.
 * \utcb{utcb}
 * \param tag      Descriptor for the message to be sent.
 * \param timeout  Timeout pair (see #l4_timeout_t) only send part is relevant.
 *
 * \return Syscall return tag for the send-only operation, this means there
 *         is no return value except #L4_MSGTAG_ERROR indicating success or
 *         failure of the send operation. Use l4_ipc_error() to check for
 *         errors and **do not** use l4_error().
 *
 * A message is sent to the destination object. There is no receive phase
 * included. The invoker continues working after sending the message.
 *
 * \note This is a special-purpose message transfer. Objects usually support
 *       only invocation via l4_ipc_call() consisting of a send phase and a
 *       receive phase for returning the result of the object invocation. For
 *       example, l4_icu_unmask(), l4_icu_mask() and l4_irq_trigger() use
 *       send-only IPC operations for object invocation.
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_send(l4_cap_idx_t dest, l4_utcb_t *utcb, l4_msgtag_t tag,
            l4_timeout_t timeout) L4_NOTHROW;


/**
 * Wait for an incoming message from any possible sender.
 * \ingroup l4_ipc_api
 *
 * \utcb{utcb}
 * \param[out] label    Label assigned to the source object (IPC gate or IRQ).
 * \param      timeout  Timeout pair (see #l4_timeout_t, only the receive part
 *                      is used).
 *
 * \return  return tag
 *
 * This operation does an open wait, and therefore needs no capability to
 * denote the possible source of a message. This means the calling thread
 * waits for an incoming message from any possible source.
 * There is no send phase included in this operation.
 *
 * The usual usage of this function is to call that function when entering a
 * server loop in a user-level server that implements user-level objects,
 * see also #l4_ipc_reply_and_wait().
 *
 * \note In case of multiple senders trying to send to the thread performing
 *       this system call, the thread receives from a sender with the highest
 *       priority. In this respect, IRQ sources have the highest priority 255.
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_wait(l4_utcb_t *utcb, l4_umword_t *label,
            l4_timeout_t timeout) L4_NOTHROW;


/**
 * Wait for a message from a specific source.
 * \ingroup l4_ipc_api
 *
 * \param object   Object to receive a message from. A value of #L4_INVALID_CAP
 *                 denotes the current thread. It could be used for sleeping
 *                 without busy waiting for the time specified in the \c rcv
 *                 part of the \c timeout parameter.
 * \utcb{utcb}
 * \param timeout  Timeout pair (see #l4_timeout_t, only the receive part
 *                 matters).
 *
 * \return  result tag.
 *
 * This operation waits for a message from the specified object. Messages from
 * other sources are not accepted by this operation. The operation does not
 * include a send phase, this means no message is sent to the object.
 *
 * \note This operation is usually used to receive messages from a specific IRQ
 *       or thread. However, it is not common to use this operation for normal
 *       applications.
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_receive(l4_cap_idx_t object, l4_utcb_t *utcb,
               l4_timeout_t timeout) L4_NOTHROW;

/**
 * Object call (usual invocation).
 * \ingroup l4_ipc_api
 *
 * \param object   Capability selector for the object to call. A value of
 *                 #L4_INVALID_CAP denotes the current thread and will abort
 *                 the IPC after the time specified in the \c snd part of the
 *                 \c timeout parameter has expired.
 * \utcb{utcb}
 * \param tag      Message tag to describe the message to be sent.
 * \param timeout  Timeout pair for send an receive phase (see #l4_timeout_t).
 *
 * \return  result tag
 *
 * A message is sent to the object and the invoker waits for a reply from the
 * object. Messages from other sources are not accepted.
 *
 * \note The send-to-receive transition needs no time, the object can reply
 *       with a send timeout of zero.
 *
 * \note If a finite receive timeout is specified, the IPC receive operation
 *       could abort before the partner was able to send the reply message.
 *       Under certain circumstances the partner may still have the temporary
 *       reply capability to the calling thread and may use this capability to
 *       reply to the caller at a later, unexpected time specifying an arbitrary
 *       IPC label. This case is relevant for servers which call another,
 *       possibly untrusted, server while serving a client request.
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_call(l4_cap_idx_t object, l4_utcb_t *utcb, l4_msgtag_t tag,
            l4_timeout_t timeout) L4_NOTHROW;


/**
 * Reply and wait operation (uses the *reply* capability).
 * \ingroup l4_ipc_api
 *
 * \utcb{utcb}
 * \param      tag      Describes the message to be sent as reply.
 * \param[out] label    Label assigned to the source object of the received
 *                      message.
 * \param      timeout  Timeout pair (see #l4_timeout_t).
 *
 * \return  result tag
 *
 * A message is sent to the previous caller using the implicit reply
 * capability. Afterwards the invoking thread waits for a message from any
 * source.
 * \note This is the standard server operation: it sends a reply to the actual
 *       client and waits for the next incoming request, which may come from
 *       any other client.
 *
 * \note In case of multiple senders trying to send to the thread performing
 *       this system call, the thread receives from a sender with the highest
 *       priority. In this respect, IRQ sources have the highest priority 255.
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_reply_and_wait(l4_utcb_t *utcb, l4_msgtag_t tag,
                      l4_umword_t *label, l4_timeout_t timeout) L4_NOTHROW;

/**
 * Send a message and do an open wait.
 * \ingroup l4_ipc_api
 *
 * \param      dest     Object to send a message to. A value of #L4_INVALID_CAP
 *                      denotes the current thread and will abort the IPC after
 *                      the time specified in the \c snd part of the \c timeout
 *                      parameter has expired.
 * \utcb{utcb}
 * \param      tag      Describes the message that shall be sent.
 * \param[out] label    Label assigned to the source object of the receive
 *                      phase.
 * \param      timeout  Timeout pair (see #l4_timeout_t).
 *
 * \return  result tag
 *
 * A message is sent to the destination object and the invoking thread waits
 * for a reply from any source.
 *
 * \note This is a special-purpose operation and shall not be used in general
 *       applications.
 *
 * \note In case of multiple senders trying to send to the thread performing
 *       this system call, the thread receives from a sender with the highest
 *       priority. In this respect, IRQ sources have the highest priority 255.
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_send_and_wait(l4_cap_idx_t dest, l4_utcb_t *utcb, l4_msgtag_t tag,
                     l4_umword_t *label, l4_timeout_t timeout) L4_NOTHROW;

/**
 * \defgroup l4_ipc_rt_api Realtime API
 * \ingroup l4_ipc_api
 * \internal
 */

#if 0
/**
 * Wait for next period.
 * \ingroup l4_ipc_rt_api
 *
 * \utcb{utcb}
 * \param label    Label
 * \param timeout  IPC timeout (see #l4_ipc_timeout).
 *
 * \return result tag
 */
L4_INLINE l4_msgtag_t
l4_ipc_wait_next_period(l4_utcb_t *utcb,
                        l4_umword_t *label,
                        l4_timeout_t timeout);

#endif

/**
 * Generic L4 object invocation.
 * \ingroup l4_ipc_api
 *
 * \param      dest     Destination object. #L4_INVALID_CAP denotes the current
 *                      thread. An IPC to the current thread will always abort
 *                      after the specified timeout and can be used for
 *                      sleeping without busy waiting.
 * \utcb{utcb}
 * \param      flags    Invocation flags (see #l4_syscall_flags_t).
 * \param      slabel   Send label if applicable (may be seen by the receiver).
 * \param      tag      Sending message tag.
 * \param[out] rlabel   Receiving label.
 * \param      timeout  Timeout pair (see #l4_timeout_t).
 *
 * \return return tag
 *
 * \see \ref l4re_concepts_ipc
 */
L4_ALWAYS_INLINE l4_msgtag_t
l4_ipc(l4_cap_idx_t dest,
       l4_utcb_t *utcb,
       l4_umword_t flags,
       l4_umword_t slabel,
       l4_msgtag_t tag,
       l4_umword_t *rlabel,
       l4_timeout_t timeout) L4_NOTHROW;

/**
 * Sleep for an amount of time.
 * \ingroup l4_ipc_api
 *
 * \param timeout  Timeout pair (see #l4_timeout_t, the receive part matters).
 *
 * \return  error code:
 *          - #L4_IPC_RETIMEOUT: success
 *          - #L4_IPC_RECANCELED woken up by a different thread
 *            (l4_thread_ex_regs()).
 *
 * The invoking thread waits until the timeout is expired or the wait was
 * aborted by another thread by l4_thread_ex_regs().
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_sleep(l4_timeout_t timeout) L4_NOTHROW;

/**
 * Sleep for a certain amount of milliseconds.
 * \ingroup l4_ipc_api
 *
 * \param ms  Number of milliseconds to wait.
 *
 * \return  error code:
 *          - #L4_IPC_RETIMEOUT: success
 *          - #L4_IPC_RECANCELED woken up by a different thread
 *            (l4_thread_ex_regs()).
 *
 * The invoking thread waits until the timeout is expired or the wait was
 * aborted by another thread by l4_thread_ex_regs().
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_sleep_ms(l4_uint32_t ms) L4_NOTHROW;

/**
 * Sleep for a certain amount of microseconds.
 * \ingroup l4_ipc_api
 *
 * \param us  Number of microseconds to wait.
 *
 * \return  error code:
 *          - #L4_IPC_RETIMEOUT: success
 *          - #L4_IPC_RECANCELED woken up by a different thread
 *            (l4_thread_ex_regs()).
 *
 * The invoking thread waits until the timeout is expired or the wait was
 * aborted by another thread by l4_thread_ex_regs().
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE l4_msgtag_t
l4_ipc_sleep_us(l4_uint64_t us) L4_NOTHROW;

/**
 * Add a flex-page to be sent to the UTCB
 * \ingroup l4_ipc_api
 *
 * \param         snd_fpage  Flex-page.
 * \param         snd_base   Send base.
 * \param[in,out] tag        Tag to be updated. Only the number of items is
 *                           incremented in the updated tag, all other members
 *                           remain unmodified.
 *
 * \return 0 on success, negative error code otherwise
 *
 * \see \ref l4re_concepts_ipc
 */
L4_INLINE int
l4_sndfpage_add(l4_fpage_t const snd_fpage, unsigned long snd_base,
                l4_msgtag_t *tag) L4_NOTHROW;

/*
 * \internal
 * \ingroup l4_ipc_api
 */
L4_INLINE int
l4_sndfpage_add_u(l4_fpage_t const snd_fpage, unsigned long snd_base,
                  l4_msgtag_t *tag, l4_utcb_t *utcb) L4_NOTHROW;


/************************************************************************
 * Implementations
 **********************/

L4_INLINE long l4_ipc_to_errno(unsigned long ipc_error_code) L4_NOTHROW
{ return -(L4_EIPC_LO + ipc_error_code); }

L4_INLINE l4_msgtag_t
l4_ipc_call(l4_cap_idx_t dest, l4_utcb_t *utcb,
            l4_msgtag_t tag,
            l4_timeout_t timeout) L4_NOTHROW
{
  return l4_ipc(dest, utcb, L4_SYSF_CALL, 0, tag, 0, timeout);
}

L4_INLINE l4_msgtag_t
l4_ipc_reply_and_wait(l4_utcb_t *utcb, l4_msgtag_t tag,
                      l4_umword_t *label,
                      l4_timeout_t timeout) L4_NOTHROW
{
  return l4_ipc(L4_INVALID_CAP, utcb, L4_SYSF_REPLY_AND_WAIT, 0, tag, label, timeout);
}

L4_INLINE l4_msgtag_t
l4_ipc_send_and_wait(l4_cap_idx_t dest, l4_utcb_t *utcb,
                     l4_msgtag_t tag,
                     l4_umword_t *src,
                     l4_timeout_t timeout) L4_NOTHROW
{
  return l4_ipc(dest, utcb, L4_SYSF_SEND_AND_WAIT, 0, tag, src, timeout);
}

L4_INLINE l4_msgtag_t
l4_ipc_send(l4_cap_idx_t dest, l4_utcb_t *utcb,
            l4_msgtag_t tag,
            l4_timeout_t timeout) L4_NOTHROW
{
  return l4_ipc(dest, utcb, L4_SYSF_SEND, 0, tag, 0, timeout);
}

L4_INLINE l4_msgtag_t
l4_ipc_wait(l4_utcb_t *utcb, l4_umword_t *src,
            l4_timeout_t timeout) L4_NOTHROW
{
  l4_msgtag_t t;
  t.raw = 0;
  return l4_ipc(L4_INVALID_CAP, utcb, L4_SYSF_WAIT, 0, t, src, timeout);
}

L4_INLINE l4_msgtag_t
l4_ipc_receive(l4_cap_idx_t src, l4_utcb_t *utcb,
               l4_timeout_t timeout) L4_NOTHROW
{
  l4_msgtag_t t;
  t.raw = 0;
  return l4_ipc(src, utcb, L4_SYSF_RECV, 0, t, 0, timeout);
}

L4_INLINE l4_msgtag_t
l4_ipc_sleep(l4_timeout_t timeout) L4_NOTHROW
{ return l4_ipc_receive(L4_INVALID_CAP, NULL, timeout); }

L4_INLINE l4_msgtag_t
l4_ipc_sleep_ms(l4_uint32_t ms) L4_NOTHROW
{
  l4_uint64_t us = ms * 1000ULL; // cannot overflow because ms < 2^32
  return l4_ipc_sleep(l4_timeout(L4_IPC_TIMEOUT_NEVER, l4_timeout_from_us(us)));
}

L4_INLINE l4_msgtag_t
l4_ipc_sleep_us(l4_uint64_t us) L4_NOTHROW
{
  return l4_ipc_sleep(l4_timeout(L4_IPC_TIMEOUT_NEVER,
                                 l4_timeout_from_us(us)));
}

L4_INLINE l4_umword_t
l4_ipc_error(l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW
{
  if (!l4_msgtag_has_error(tag))
    return 0;
  return l4_utcb_tcr_u(utcb)->error & L4_IPC_ERROR_MASK;
}

L4_INLINE long
l4_error_u(l4_msgtag_t tag, l4_utcb_t *u) L4_NOTHROW
{
  if (l4_msgtag_has_error(tag))
    return l4_ipc_to_errno(l4_utcb_tcr_u(u)->error & L4_IPC_ERROR_MASK);

  return l4_msgtag_label(tag);
}

L4_INLINE long
l4_error(l4_msgtag_t tag) L4_NOTHROW
{
  return l4_error_u(tag, l4_utcb());
}


L4_INLINE int l4_ipc_is_snd_error(l4_utcb_t *u) L4_NOTHROW
{ return (l4_utcb_tcr_u(u)->error & 1) == 0; }

L4_INLINE int l4_ipc_is_rcv_error(l4_utcb_t *u) L4_NOTHROW
{ return l4_utcb_tcr_u(u)->error & 1; }

L4_INLINE int l4_ipc_error_code(l4_utcb_t *u) L4_NOTHROW
{ return l4_utcb_tcr_u(u)->error & L4_IPC_ERROR_MASK; }


/*
 * \internal
 * \ingroup l4_ipc_api
 */
L4_INLINE int
l4_sndfpage_add_u(l4_fpage_t const snd_fpage, unsigned long snd_base,
                  l4_msgtag_t *tag, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  int i = l4_msgtag_words(*tag) + 2 * l4_msgtag_items(*tag);

  if (i >= L4_UTCB_GENERIC_DATA_SIZE - 1)
    return -L4_ENOMEM;

  v->mr[i]     = snd_base | L4_ITEM_MAP | L4_ITEM_CONT;
  v->mr[i + 1] = snd_fpage.raw;

  *tag = l4_msgtag(l4_msgtag_label(*tag), l4_msgtag_words(*tag),
                   l4_msgtag_items(*tag) + 1, l4_msgtag_flags(*tag));
  return 0;
}

L4_INLINE int
l4_sndfpage_add(l4_fpage_t const snd_fpage, unsigned long snd_base,
                l4_msgtag_t *tag) L4_NOTHROW
{
  return l4_sndfpage_add_u(snd_fpage, snd_base, tag, l4_utcb());
}


#endif /* ! __L4SYS__INCLUDE__L4API_FIASCO__IPC_H__ */
