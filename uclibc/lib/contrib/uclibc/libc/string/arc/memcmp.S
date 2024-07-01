/*
 * Copyright (C) 2013, 2022 Synopsys, Inc. (www.synopsys.com)
 * Copyright (C) 2007 ARC International (UK) LTD
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>
#include <features.h>

#ifdef __LITTLE_ENDIAN__
#define WORD2 r2
#define SHIFT r3
#else /* BIG ENDIAN */
#define WORD2 r3
#define SHIFT r2
#endif

ENTRY(memcmp)

#if defined(__ARC700__) || defined(__ARCHS__)
	or	r12,r0,r1
	asl_s	r12,r12,30
	sub	r3,r2,1
	brls	r2,r12,.Lbytewise
	ld	r4,[r0,0]
	ld	r5,[r1,0]
	lsr.f	lp_count,r3,3
#ifdef __HS__
	/* In ARCv2 a branch can't be the last instruction in a zero overhead
	 * loop.
	 * So we move the branch to the start of the loop, duplicate it
	 * after the end, and set up r12 so that the branch isn't taken
	 *  initially.
	 */
	mov_s	r12,WORD2
	lpne	.Loop_end
	brne	WORD2,r12,.Lodd
	ld	WORD2,[r0,4]
#else
	lpne	.Loop_end
	ld_s	WORD2,[r0,4]
#endif
	ld_s	r12,[r1,4]
	brne	r4,r5,.Leven
	ld.a	r4,[r0,8]
	ld.a	r5,[r1,8]
#ifdef __HS__
.Loop_end:
	brne	WORD2,r12,.Lodd
#else
	brne	WORD2,r12,.Lodd
.Loop_end:
#endif
	asl_s	SHIFT,SHIFT,3
	bhs_s	.Last_cmp
	brne	r4,r5,.Leven
	ld	r4,[r0,4]
	ld	r5,[r1,4]
#ifdef __LITTLE_ENDIAN__
	nop_s
	; one more load latency cycle
.Last_cmp:
	xor	r0,r4,r5
	bset	r0,r0,SHIFT
	sub_s	r1,r0,1
	bic_s	r1,r1,r0
	norm	r1,r1
	b.d	.Leven_cmp
	and	r1,r1,24
.Leven:
	xor	r0,r4,r5
	sub_s	r1,r0,1
	bic_s	r1,r1,r0
	norm	r1,r1
	; slow track insn
	and	r1,r1,24
.Leven_cmp:
	asl	r2,r4,r1
	asl	r12,r5,r1
	lsr_s	r2,r2,1
	lsr_s	r12,r12,1
	j_s.d	[blink]
	sub	r0,r2,r12
	.balign	4
.Lodd:
	xor	r0,WORD2,r12
	sub_s	r1,r0,1
	bic_s	r1,r1,r0
	norm	r1,r1
	; slow track insn
	and	r1,r1,24
	asl_s	r2,r2,r1
	asl_s	r12,r12,r1
	lsr_s	r2,r2,1
	lsr_s	r12,r12,1
	j_s.d	[blink]
	sub	r0,r2,r12
#else /* BIG ENDIAN */
.Last_cmp:
	neg_s	SHIFT,SHIFT
	lsr	r4,r4,SHIFT
	lsr	r5,r5,SHIFT
	; slow track insn
.Leven:
	sub.f	r0,r4,r5
	mov.ne	r0,1
	j_s.d	[blink]
	bset.cs	r0,r0,31
.Lodd:
	cmp_s	WORD2,r12
	mov_s	r0,1
	j_s.d	[blink]
	bset.cs	r0,r0,31
#endif /* ENDIAN */
	.balign	4
.Lbytewise:
	breq	r2,0,.Lnil
	ldb	r4,[r0,0]
	ldb	r5,[r1,0]
	lsr.f	lp_count,r3
#ifdef __HS__
	mov	r12,r3
	lpne	.Lbyte_end
	brne	r3,r12,.Lbyte_odd
#else
	lpne	.Lbyte_end
#endif
	ldb_s	r3,[r0,1]
	ldb	r12,[r1,1]
	brne	r4,r5,.Lbyte_even
	ldb.a	r4,[r0,2]
	ldb.a	r5,[r1,2]
#ifdef __HS__
.Lbyte_end:
	brne	r3,r12,.Lbyte_odd
#else
	brne	r3,r12,.Lbyte_odd
.Lbyte_end:
#endif
	bcc	.Lbyte_even
	brne	r4,r5,.Lbyte_even
	ldb_s	r3,[r0,1]
	ldb_s	r12,[r1,1]
.Lbyte_odd:
	j_s.d	[blink]
	sub	r0,r3,r12
.Lbyte_even:
	j_s.d	[blink]
	sub	r0,r4,r5
.Lnil:
	j_s.d	[blink]
	mov	r0,0

#elif (__ARC64_ARCH32__)
	;; Based on Synopsys code from newlib's arc64/memcmp.S
	cmp		r2, 32
	bls.d	@.L_compare_1_bytes
	mov		r3, r0	; "r0" will be used as return value

	lsr		r12, r2, 4	; counter for 16-byte chunks
	xor		r13, r13, r13	; the mask showing inequal registers

.L_compare_16_bytes:
	ld.ab	r4, [r3, +4]
	ld.ab	r5, [r1, +4]
	ld.ab	r6, [r3, +4]
	ld.ab	r7, [r1, +4]
	ld.ab	r8, [r3, +4]
	ld.ab	r9, [r1, +4]
	ld.ab	r10, [r3, +4]
	ld.ab	r11, [r1, +4]
	xor.f	0, r4, r5
	xor.ne	r13, r13, 0b0001
	xor.f	0, r6, r7
	xor.ne	r13, r13, 0b0010
	xor.f	0, r8, r9
	xor.ne	r13, r13, 0b0100
	xor.f	0, r10, r11
	xor.ne	r13, r13, 0b1000
	brne	r13, 0, @.L_unequal_find
	dbnz	r12, @.L_compare_16_bytes

	;; Adjusting the pointers because of the extra loads in the end
	sub		r1, r1, 4
	sub		r3, r3, 4
	bmsk_s	  r2, r2, 3	; any remaining bytes to compare

.L_compare_1_bytes:
	cmp		r2, 0
	jeq.d	[blink]
	xor_s	r0, r0, r0

2:
	ldb.ab	r4, [r3, +1]
	ldb.ab	r5, [r1, +1]
	sub.f	r0, r4, r5
	jne		[blink]
	dbnz	r2, @2b
	j_s		[blink]

	;; At this point, we want to find the _first_ comparison that marked the
	;; inequality of "lhs" and "rhs"
.L_unequal_find:
	ffs		r13, r13
	asl		r13, r13, 2
	bi		[r13]
.L_unequal_r4r5:
	mov		r1, r4
	b.d		@.L_diff_byte_in_regs
	mov		r2, r5
	nop
.L_unequal_r6r7:
	mov		r1, r6
	b.d		@.L_diff_byte_in_regs
	mov		r2, r7
	nop
.L_unequal_r8r9:
	mov		r1, r8
	b.d		@.L_diff_byte_in_regs
	mov		r2, r9
	nop
.L_unequal_r10r11:
	mov		r1, r10
	mov		r2, r11

	;; fall-through
	;; If we're here, that means the two operands are not equal.
.L_diff_byte_in_regs:
	xor		r0, r1, r2
	ffs		r0, r0
	and		r0, r0, 0x18
	lsr		r1, r1, r0
	lsr		r2, r2, r0
	bmsk_s	r1, r1, 7
	bmsk_s	r2, r2, 7
	j_s.d	[blink]
	sub		r0, r1, r2

#else
#error "Unsupported ARC CPU type"
#endif

END(memcmp)
libc_hidden_def(memcmp)

#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(memcmp,bcmp)
#endif
