/**
 * \file
 * L4 kernel event tracing
 * \ingroup api_calls
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

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

L4_INLINE l4_umword_t
fiasco_tbuf_log(const char *text)
{
  enum { TBUF_LOG = L4_KDEBUG_GROUP_TRACE + 0x01 };
  return l4_error(__kdebug_text(TBUF_LOG, text, __builtin_strlen(text)));
}

L4_INLINE l4_umword_t
fiasco_tbuf_log_3val(const char *text, l4_umword_t v1, l4_umword_t v2,
                     l4_umword_t v3)
{
  enum { TBUF_LOG_3VAL = L4_KDEBUG_GROUP_TRACE + 0x04 };
  return l4_error(__kdebug_3_text(TBUF_LOG_3VAL, text,
                                  __builtin_strlen(text), v1, v2, v3));
}

L4_INLINE void
fiasco_tbuf_clear(void)
{
  enum { TBUF_CLEAR = L4_KDEBUG_GROUP_TRACE + 0x02 };
  __kdebug_op(TBUF_CLEAR);
}

L4_INLINE void
fiasco_tbuf_dump(void)
{
  enum { TBUF_DUMP = L4_KDEBUG_GROUP_TRACE + 0x03 };
  __kdebug_op(TBUF_DUMP);
}

L4_INLINE l4_umword_t
fiasco_tbuf_log_binary(const unsigned char *data)
{
  enum { TBUF_LOG_BIN = L4_KDEBUG_GROUP_TRACE + 0x08 };
  return l4_error(__kdebug_text(TBUF_LOG_BIN, (const char *)data, 24));
}

