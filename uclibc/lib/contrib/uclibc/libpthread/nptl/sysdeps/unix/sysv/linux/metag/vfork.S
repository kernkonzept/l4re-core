/* Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <tcb-offsets.h>
#include <asm/unistd.h>

#ifdef __PIC__
#define __VFORK_METAG_LOAD_TP ___metag_load_tp@PLT
#else
#define __VFORK_METAG_LOAD_TP ___metag_load_tp
#endif

/* Save the PID value.  */
#define SAVE_PID \
	SETL	[A0StP++], D0FrT, D1RtP; \
	CALLR	D1RtP, __VFORK_METAG_LOAD_TP; \
	SUB	D0Re0, D0Re0, #TLS_PRE_TCB_SIZE; \
	GETD	D0FrT, [D0Re0 + #PID]; \
	NEGS	D0FrT, D0FrT; \
	BNZ	1f; \
	MOVT	D0FrT, #0x8000; \
1:	SETD	[D0Re0 + #PID], D0FrT; \
	GETL	D0FrT, D1RtP, [--A0StP];

#define RESTORE_PID \
	CMP 	D0Re0, #0; \
	BEQ	1f; \
	MSETL	[A0StP++], D0Re0, D0FrT; \
	CALLR	D1RtP, __VFORK_METAG_LOAD_TP; \
	SUB	D0Re0, D0Re0, #TLS_PRE_TCB_SIZE; \
	GETD	D0FrT, [D0Re0 + #PID]; \
	NEG	D0FrT, D0FrT; \
	MOVT	D1Re0, #0x8000; \
	CMP 	D0FrT, D1Re0; \
	XOREQ	D0FrT, D0FrT, D0FrT; \
	SETD	[D0Re0 + #PID], D0FrT; \
	GETL	D0FrT, D1RtP, [--A0StP]; \
	GETL	D0Re0, D1Re0, [--A0StP]; \
1:

#include <../../../../../../../libc/sysdeps/linux/metag/vfork.S>
