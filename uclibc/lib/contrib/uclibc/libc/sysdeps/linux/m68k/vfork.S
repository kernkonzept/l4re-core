/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif

#define IMM #

	.text
	.align 2
	.globl	__vfork
	.hidden	__vfork
	.type	__vfork,@function

__vfork:
	movl	%sp@+, %a1               /* save the return address for later */
	movl	IMM __NR_vfork,%d0
	trap	#0
	movl	%a1, -(%sp)

	cmpil	#-4096,%d0
	blss	1f

	neg.l	%d0
#ifndef __PIC__					/* needs handling as the other archs */
	movl	errno, %a0
#else
	movl	errno@GOT(%a5), %a0
#endif
	movl	%d0, %a0@
	move.l	#-1, %d0

1:
	move.l	%d0, %a0
	rts

.size __vfork,.-__vfork
weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
