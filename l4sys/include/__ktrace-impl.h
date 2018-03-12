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

#include <l4/sys/types.h>
#include <l4/sys/kdebug.h>

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

L4_INLINE l4_tracebuffer_status_t *
fiasco_tbuf_get_status(void)
{
  return 0;
  /* Not implemented */
}

L4_INLINE l4_addr_t
fiasco_tbuf_get_status_phys(void)
{
  return ~0UL;
}

L4_INLINE l4_umword_t
fiasco_tbuf_log(const char *text)
{
  enum { TBUF_LOG = 0x201 };
  return l4_error(__kdebug_text(TBUF_LOG, text, __builtin_strlen(text)));
}

L4_INLINE l4_umword_t
fiasco_tbuf_log_3val(const char *text, l4_umword_t v1, l4_umword_t v2,
                     l4_umword_t v3)
{
  enum { TBUF_LOG_3VAL = 0x204 };
  return l4_error(__kdebug_3_text(TBUF_LOG_3VAL, text,
                                  __builtin_strlen(text), v1, v2, v3));
}

L4_INLINE void
fiasco_tbuf_clear(void)
{
  enum { TBUF_CLEAR = 0x202 };
  __kdebug_op(TBUF_CLEAR);
}

L4_INLINE void
fiasco_tbuf_dump(void)
{
  enum { TBUF_DUMP = 0x203 };
  __kdebug_op(TBUF_DUMP);
}

L4_INLINE l4_umword_t
fiasco_tbuf_log_binary(const unsigned char *data)
{
  enum { TBUF_LOG_BIN = 0x208 };
  return l4_error(__kdebug_text(TBUF_LOG_BIN, (const char *)data, 24));
}

