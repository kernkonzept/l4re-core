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

#ifndef __KDEBUG_H__
#define __KDEBUG_H__

#include <l4/sys/compiler.h>
#include <l4/sys/consts.h>
#include <l4/sys/ipc.h>


L4_INLINE void
enter_kdebug(char const *text) L4_NOTHROW;

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
                              (len + sizeof(l4_umword_t) -1)
                              / sizeof(l4_umword_t) + 2,
                              0, 0),
                    L4_IPC_NEVER);
  __builtin_memcpy(mr, &store, sizeof(*mr));
  return res;
}

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
                             (len + sizeof(l4_umword_t) -1)
                             / sizeof(l4_umword_t) + 5,
                             0, 0),
                    L4_IPC_NEVER);
  __builtin_memcpy(mr, &store, sizeof(*mr));
  return res;
}

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

L4_INLINE void outnstring(char const *text, unsigned len)
{ __kdebug_text(L4_KDEBUG_OUTNSTRING, text, len); }

L4_INLINE void outstring(char const *text)
{ outnstring(text, __builtin_strlen(text)); }


L4_INLINE void outchar(char c)
{
  __kdebug_op_1(L4_KDEBUG_OUTCHAR, c);
}

L4_INLINE void outhex32(l4_uint32_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX32, number);
}

L4_INLINE void outhex20(l4_uint32_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX20, number);
}

L4_INLINE void outhex16(l4_uint16_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX16, number);
}

L4_INLINE void outhex12(l4_uint16_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX12, number);
}

L4_INLINE void outhex8(l4_uint8_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTHEX8, number);
}

L4_INLINE void outdec(l4_mword_t number)
{
  __kdebug_op_1(L4_KDEBUG_OUTDEC, number);
}

L4_INLINE int l4kd_inchar(void)
{
  return -1;
}

/**
 * Show message with L4 kernel debugger, but do not enter debugger
 * \ingroup l4_debugger_api
 * \hideinitializer
 *
 * \param text  Text to be shown
 */
L4_INLINE void kd_display(char const *text)
{ outstring(text); }

L4_INLINE void ko(char c)
{
  outchar(c);
}

L4_INLINE void
fiasco_profile_start(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_profile_stop_and_dump(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_profile_stop(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_watchdog_enable(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_watchdog_takeover(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_watchdog_giveback(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_watchdog_touch(void) L4_NOTHROW
{}

L4_INLINE void
fiasco_watchdog_disable(void) L4_NOTHROW
{}

#endif //__KDEBUG_H__
