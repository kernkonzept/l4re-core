/* Copyright (C) 2012 Free Software Foundation, Inc.
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

	.text
	.align 4

	.globl pthread_spin_lock
pthread_spin_lock:

	entry	a1, 16

	movi	a3, 0
	wsr 	a3, scompare1
	movi	a3, 1
1:	s32c1i	a3, a2, 0
	bnez	a3, 1b
	movi	a2, 0

	retw

	.type pthread_spin_lock, @function
	.size pthread_spin_lock, .-pthread_spin_lock
