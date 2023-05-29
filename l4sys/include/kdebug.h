/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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

/**
 * \file
 * Functionality for invoking the kernel debugger.
 * \ingroup l4_debugger_api
 */

#ifndef __KDEBUG_H__
#define __KDEBUG_H__

#include <l4/sys/compiler.h>
#include <l4/sys/consts.h>
#include <l4/sys/ipc.h>


L4_INLINE void
enter_kdebug(char const *text) L4_NOTHROW;

/**
 * Op-codes for operations that can be invoked on the base debugger capability.
 * See also __ktrace-impl.h for additional op-codes.
 */
enum l4_kdebug_ops_t
{
  L4_KDEBUG_ENTER      = 0,
  L4_KDEBUG_OUTCHAR    = 1,
  L4_KDEBUG_OUTNSTRING = 2,
  L4_KDEBUG_OUTHEX32   = 3,
  L4_KDEBUG_OUTHEX20   = 4,
  L4_KDEBUG_OUTHEX16   = 5,
  L4_KDEBUG_OUTHEX12   = 6,
  L4_KDEBUG_OUTHEX8    = 7,
  L4_KDEBUG_OUTDEC     = 8,
};


/**
 * Invoke a nullary operation on the base debugger capability.
 *
 * \param op  Nullary operation code from #l4_kdebug_ops_t or a value above
 *            0x200 used by the kernel trace buffer implementation
 *            (__ktrace-impl.h).
 *
 * \retval  Message tag returned from the IPC on the base debugger capability.
 */
L4_INLINE l4_msgtag_t
__kdebug_op(unsigned op) L4_NOTHROW
{
  l4_msgtag_t res;
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);
  l4_umword_t mr0 = mr->mr[0];

  mr->mr[0] = op;
  res = l4_ipc_call(L4_BASE_DEBUGGER_CAP, u,
                    l4_msgtag(L4_PROTO_DEBUGGER, 1, 0, 0),
                    L4_IPC_NEVER);
  mr->mr[0] = mr0;
  return res;
}

/**
 * Invoke a text output operation on the base debugger capability.
 *
 * \param op    Text output operation code from #l4_kdebug_ops_t or a value
 *              above 0x200 used by the kernel trace buffer implementation
 *              (__ktrace-impl.h).
 * \param text  Output string.
 * \param len   Length of the output string. The maximum length is limited
 *              to #L4_UTCB_GENERIC_DATA_SIZE&nbsp;-&nbsp;2 machine words.
 *              Output strings longer than this limit will be cropped.
 *
 * \retval      Message tag returned from the IPC on the base debugger
 *              capability.
 */
L4_INLINE l4_msgtag_t
__kdebug_text(unsigned op, char const *text, unsigned len) L4_NOTHROW
{
  l4_msg_regs_t store;
  l4_msgtag_t res;
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);

  if (len > (sizeof(store) - (2 * sizeof(l4_umword_t))))
    len = sizeof(store) - (2 * sizeof(l4_umword_t));

  __builtin_memcpy(&store, mr, sizeof(store));
  mr->mr[0] = op;
  mr->mr[1] = len;
  __builtin_memcpy(&mr->mr[2], text, len);
  res = l4_ipc_call(L4_BASE_DEBUGGER_CAP, u,
                    l4_msgtag(L4_PROTO_DEBUGGER,
                              l4_bytes_to_mwords(len) + 2, 0, 0),
                    L4_IPC_NEVER);
  __builtin_memcpy(mr, &store, sizeof(*mr));
  return res;
}

/**
 * Invoke a text output operation with 3 additional machine word arguments on
 * the base debugger capability.
 *
 * \param op    Text output operation code from #l4_kdebug_ops_t or a value
 *              above 0x200 used by the kernel trace buffer implementation
 *              (__ktrace-impl.h).
 * \param text  Output string.
 * \param len   Length of the output string. The maximum length is limited
 *              to #L4_UTCB_GENERIC_DATA_SIZE&nbsp;-&nbsp;5 machine words.
 *              Output strings longer than this limit will be cropped.
 * \param v1    First machine word argument.
 * \param v2    Second machine word argument.
 * \param v3    Third machine word argument.
 *
 * \retval      Message tag returned from the IPC on the base debugger
 *              capability.
 */
L4_INLINE l4_msgtag_t
__kdebug_3_text(unsigned op, char const *text, unsigned len,
                l4_umword_t v1, l4_umword_t v2, l4_umword_t v3) L4_NOTHROW
{
  l4_msg_regs_t store;
  l4_msgtag_t res;
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);

  if (len > (sizeof(store) - (5 * sizeof(l4_umword_t))))
    len = sizeof(store) - (5 * sizeof(l4_umword_t));

  __builtin_memcpy(&store, mr, sizeof(store));
  mr->mr[0] = op;
  mr->mr[1] = v1;
  mr->mr[2] = v2;
  mr->mr[3] = v3;
  mr->mr[4] = len;
  __builtin_memcpy(&mr->mr[5], text, len);
  res = l4_ipc_call(L4_BASE_DEBUGGER_CAP, u,
                    l4_msgtag(L4_PROTO_DEBUGGER,
                              l4_bytes_to_mwords(len) + 5, 0, 0),
                    L4_IPC_NEVER);
  __builtin_memcpy(mr, &store, sizeof(*mr));
  return res;
}

/**
 * Invoke an unary operation on the base debugger capability.
 *
 * \param op   Unary operation code from #l4_kdebug_ops_t or a value above
 *             0x200 used by the kernel trace buffer implementation
 *             (__ktrace-impl.h).
 * \param val  Machine word argument to the unary operation.
 *
 * \retval  Message tag returned from the IPC on the base debugger capability.
 */
L4_INLINE l4_msgtag_t
__kdebug_op_1(unsigned op, l4_mword_t val) L4_NOTHROW
{
  l4_umword_t m[2];
  l4_msgtag_t res;
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);

  m[0] = mr->mr[0];
  m[1] = mr->mr[1];
  mr->mr[0] = op;
  mr->mr[1] = val;
  res = l4_ipc_call(L4_BASE_DEBUGGER_CAP, u,
                    l4_msgtag(L4_PROTO_DEBUGGER, 2, 0, 0),
                    L4_IPC_NEVER);
  mr->mr[0] = m[0];
  mr->mr[1] = m[1];
  return res;
}

/**
 * Enter the kernel debugger.
 *
 * \param text    Optional message displayed by the kernel debugger when
 *                entered.
 *
 * Enter the kernel debugger, if configured. An optional message can be passed
 * to the kernel debugger which is printed upon the entering of the debugger.
 */
L4_INLINE void enter_kdebug(char const *text) L4_NOTHROW
{
  /* special case, enter without any text and use of the UTCB */
  if (!text)
    {
      l4_ipc_call(L4_BASE_DEBUGGER_CAP, 0,
                  l4_msgtag(L4_PROTO_DEBUGGER, 0, 0, 0),
                  L4_IPC_NEVER);
      return;
    }

  __kdebug_text(L4_KDEBUG_ENTER, text, __builtin_strlen(text));
}

/**
 * Output a fixed-length string via the kernel debugger.
 *
 * \param text    Beginning of the output string.
 * \param len     Length of the output string. The maximum length is limited
 *                to #L4_UTCB_GENERIC_DATA_SIZE&nbsp;-&nbsp;2 machine words.
 *                Output strings longer than this limit will be cropped.
 */
L4_INLINE void outnstring(char const *text, unsigned len)
{ __kdebug_text(L4_KDEBUG_OUTNSTRING, text, len); }

/**
 * Output a string via the kernel debugger.
 *
 * \param text    Beginning of the output string. The maximum length of the
 *                output string is limited to
 *                #L4_UTCB_GENERIC_DATA_SIZE&nbsp;-&nbsp;2 machine words.
 *                Output strings longer than this limit will be cropped.
 */
L4_INLINE void outstring(char const *text)
{ outnstring(text, __builtin_strlen(text)); }

/**
 * Output a single character via the kernel debugger.
 *
 * \param c    Output character.
 */
L4_INLINE void outchar(char c)
{
  __kdebug_op_1(L4_KDEBUG_OUTCHAR, c);
}

/**
 * Output a hexadecimal unsigned machine word via the kernel debugger.
 *
 * \param number    Output machine word.
 *
 * If the machine word is 64 bits long, it is printed non-atomically as two
 * 32-bit numbers.
 */
L4_INLINE void outumword(l4_umword_t number)
{
  if (sizeof(l4_umword_t) == sizeof(l4_uint64_t))
    __kdebug_op_1(L4_KDEBUG_OUTHEX32, (l4_uint64_t)number >> 32);

  __kdebug_op_1(L4_KDEBUG_OUTHEX32, number);
}

/**
 * Output a 64-bit unsigned hexadecimal number via the kernel debugger.
 *
 * \param number    Output 64-bit number.
 *
 * The two 32-bit halves are printed non-atomically.
 */
L4_INLINE void outhex64(l4_uint64_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX32, number >> 32);
  __kdebug_op_1(L4_KDEBUG_OUTHEX32, number);
}

/**
 * Output a 32-bit unsigned hexadecimal number via the kernel debugger.
 *
 * \param number    Output 32-bit number.
 */
L4_INLINE void outhex32(l4_uint32_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX32, number);
}

/**
 * Output a 20-bit unsigned hexadecimal number via the kernel debugger.
 *
 * \param number    Output 20-bit number. Only the 20 LSB bits are used.
 */
L4_INLINE void outhex20(l4_uint32_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX20, number);
}

/**
 * Output a 16-bit unsigned hexadecimal number via the kernel debugger.
 *
 * \param number    Output 16-bit number.
 */
L4_INLINE void outhex16(l4_uint16_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX16, number);
}

/**
 * Output a 12-bit unsigned hexadecimal number via the kernel debugger.
 *
 * \param number    Output 12-bit number. Only the 12 LSB bits are used.
 */
L4_INLINE void outhex12(l4_uint16_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX12, number);
}

/**
 * Output an 8-bit unsigned hexadecimal number via the kernel debugger.
 *
 * \param number    Output 8-bit number.
 */
L4_INLINE void outhex8(l4_uint8_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX8, number);
}

/**
 * Output a decimal unsigned machine word via the kernel debugger.
 *
 * \param number    Output machine word.
 */
L4_INLINE void outdec(l4_mword_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTDEC, number);
}

#endif //__KDEBUG_H__
