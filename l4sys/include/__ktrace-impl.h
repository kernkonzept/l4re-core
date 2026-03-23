/**
 * \file
 * L4 kernel event tracing
 * \ingroup api_calls_fiasco
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/task.h>

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

L4_INLINE l4_msgtag_t
fiasco_tbuf_validate(void)
{ return l4_task_cap_valid(L4_BASE_TASK_CAP, L4_BASE_DEBUGGER_CAP); }

L4_INLINE l4_umword_t
fiasco_tbuf_log(const char *text)
{
  enum { TBUF_LOG = L4_KDEBUG_GROUP_TRACE + 0x01 };
  return l4_error(__l4_kdebug_text(TBUF_LOG, text, __builtin_strlen(text)));
}

L4_INLINE l4_umword_t
fiasco_tbuf_log_3val(const char *text, l4_umword_t v1, l4_umword_t v2,
                     l4_umword_t v3)
{
  enum { TBUF_LOG_3VAL = L4_KDEBUG_GROUP_TRACE + 0x04 };
  return l4_error(__l4_kdebug_3_text(TBUF_LOG_3VAL, text,
                                     __builtin_strlen(text), v1, v2, v3));
}

L4_INLINE void
fiasco_tbuf_clear(void)
{
  enum { TBUF_CLEAR = L4_KDEBUG_GROUP_TRACE + 0x02 };
  __l4_kdebug_op(TBUF_CLEAR);
}

L4_INLINE void
fiasco_tbuf_dump(void)
{
  enum { TBUF_DUMP = L4_KDEBUG_GROUP_TRACE + 0x03 };
  __l4_kdebug_op(TBUF_DUMP);
}

L4_INLINE l4_umword_t
fiasco_tbuf_log_binary(const unsigned char *data)
{
  enum { TBUF_LOG_BIN = L4_KDEBUG_GROUP_TRACE + 0x08 };
  return l4_error(__l4_kdebug_text(TBUF_LOG_BIN, (const char *)data, 24));
}

L4_INLINE l4_msgtag_t
fiasco_tbuf_map_status(l4_fpage_t *ku_mem)
{
  enum { TBUF_MAP_STATUS = L4_KDEBUG_GROUP_TRACE + 0x10 };

  l4_utcb_t *utcb = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);
  l4_msgtag_t ret;

  mr->mr[0] = TBUF_MAP_STATUS;
  mr->mr[1] = ku_mem->raw;
  ret = l4_ipc_call(L4_BASE_DEBUGGER_CAP, utcb,
                    l4_msgtag(L4_PROTO_DEBUGGER, 2, 0, 0), L4_IPC_NEVER);
  if (!l4_msgtag_has_error(ret))
    ku_mem->raw = mr->mr[0];

  return ret;
}

L4_INLINE l4_msgtag_t
fiasco_tbuf_map_slots(l4_fpage_t *ku_mem)
{
  enum { TBUF_MAP_SLOTS = L4_KDEBUG_GROUP_TRACE + 0x11 };

  l4_utcb_t *utcb = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);
  l4_msgtag_t ret;

  mr->mr[0] = TBUF_MAP_SLOTS;
  mr->mr[1] = ku_mem->raw;
  ret = l4_ipc_call(L4_BASE_DEBUGGER_CAP, utcb,
                    l4_msgtag(L4_PROTO_DEBUGGER, 2, 0, 0), L4_IPC_NEVER);
  if (!l4_msgtag_has_error(ret))
    ku_mem->raw = mr->mr[0];

  return ret;
}
