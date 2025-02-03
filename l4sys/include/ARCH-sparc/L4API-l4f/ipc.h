/**
 * \internal
 * \file
 * \brief   L4 IPC System Calls, Sparc
 */
/*
 * (c) 2010    Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *             Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/sys/ipc.h>

#ifdef __GNUC__

#include <l4/sys/compiler.h>
#include <l4/sys/syscall_defs.h>

L4_INLINE l4_msgtag_t
l4_ipc(l4_cap_idx_t dest, l4_utcb_t *utcb,
       l4_umword_t flags,
       l4_umword_t slabel,
       l4_msgtag_t tag,
       l4_umword_t *rlabel,
       l4_timeout_t timeout) L4_NOTHROW
{
	l4_msgtag_t t;
	(void)dest;
	(void)utcb;
	(void)flags;
	(void)slabel;
	(void)tag;
	(void)rlabel;
	(void)timeout;
	t.raw = ~0;
	return t;
}

#endif //__GNUC__

