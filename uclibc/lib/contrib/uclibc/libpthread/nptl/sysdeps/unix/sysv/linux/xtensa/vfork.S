/* Copyright (C) 2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <tls.h>

/*
  Save the PID value, save 0x80000000 if PID was 0.
  Registers a2 and a3 are available; ar should return the PID and as threadptr
 */

#define SAVE_PID(pid,tp,ar,as)						\
	rur	tp, threadptr;						\
	movi	ar, TLS_PRE_TCB_SIZE;					\
	sub	tp, tp, ar;						\
	l32i	pid, tp, PID;						\
	neg	ar, pid;						\
	movi	as, 0x80000000;						\
	moveqz	ar, as, ar;						\
	s32i	ar, tp, PID;						\

/*
  Restore the PID value, restore to 0 if saved value was 0x80000000
  Return value from the syscall is in a2.
 */
#define RESTORE_PID(pid,tp,res)						\
	beqz	res, 1f;						\
	s32i	pid, tp, PID;						\
1:

/*
  Special version for call12, where we don't have enough registers
  available to preserve the original PID.
 */
#define RESTORE_PID12(ar, as, at)					\
	rur	as, threadptr;						\
	movi	ar, TLS_PRE_TCB_SIZE;					\
	sub	as, as, ar;						\
	l32i	ar, as, PID;						\
	movi	at, 0x80000000;						\
	sub	at, at, ar;						\
	neg	ar, ar;							\
	moveqz	ar, at, at;						\
	s32i	ar, as, PID;

#include <libc/sysdeps/linux/xtensa/vfork.S>
