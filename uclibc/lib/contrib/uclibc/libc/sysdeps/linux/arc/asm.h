/*
 * Copyright (C) 2022, Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _ARC_ASM_H
#define _ARC_ASM_H

/*
 * Some 16-bit instructions were excluded from the ARCv3 ISA
 * the following macros are introduced to handle these changes in one place.
 * This will allow not to change existing ARCv2 code and use 16-bit versions
 * of instructions for ARCv2 and replace them with 32-bit vesrions for ARCv3
 */

#if defined (__ARC64_ARCH32__)

.macro PUSHR reg
	push	\reg
.endm

.macro PUSHR_S reg
	push	\reg
.endm

.macro POPR reg
	pop	\reg
.endm

.macro POPR_S reg
	pop	\reg
.endm

.macro SUBR_S dst,src1,src2
	sub	\dst, \src1, \src2
.endm

.macro ADDR_S dst,src1,src2
	add	\dst, \src1, \src2
.endm

.macro ASRR_S dst,src1,src2
	asr	\dst, \src1, \src2
.endm

.macro ASLR_S dst,src1,src2
	asl	\dst, \src1, \src2
.endm

#elif defined (__ARC64_ARCH64__)

# error ARCv3 64-bit is not supported by uClibc-ng

#else /* ARCHS || ARC700 */

.macro PUSHR reg
	push	\reg
.endm

.macro PUSHR_S reg
	push_s	\reg
.endm

.macro POPR reg
	pop	\reg
.endm

.macro POPR_S reg
	pop_s	\reg
.endm

.macro SUBR_S dst,src1,src2
	sub_s	\dst, \src1, \src2
.endm

.macro ADDR_S dst,src1,src2
	add_s	\dst, \src1, \src2
.endm

.macro ASRR_S dst,src1,src2
	asr_s	\dst, \src1, \src2
.endm

.macro ASLR_S dst,src1,src2
	asl_s	\dst, \src1, \src2
.endm

#endif

#endif /* _ARC_ASM_H  */
