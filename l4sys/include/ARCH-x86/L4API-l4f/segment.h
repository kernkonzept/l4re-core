#include_next <l4/sys/segment.h>

/**
 * \file
 * \brief   l4f specific segment manipulation
 * \ingroup api_calls_fiasco
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
#ifndef __L4_SYS__ARCH_X86__L4API_L4F__SEGMENT_H__
#define __L4_SYS__ARCH_X86__L4API_L4F__SEGMENT_H__

#include <l4/sys/compiler.h>

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

L4_INLINE long
fiasco_gdt_set(l4_cap_idx_t thread, void *desc, unsigned int size,
	       unsigned int entry_number_start, l4_utcb_t *utcb)
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_THREAD_X86_GDT_OP;
  l4_utcb_mr_u(utcb)->mr[1] = entry_number_start;
  __builtin_memcpy(&l4_utcb_mr_u(utcb)->mr[2], desc, size);
  return l4_error_u(l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 2 + (size >> 2), 0, 0), L4_IPC_NEVER), utcb);
}

#endif /* ! __L4_SYS__ARCH_X86__L4API_L4F__SEGMENT_H__ */
