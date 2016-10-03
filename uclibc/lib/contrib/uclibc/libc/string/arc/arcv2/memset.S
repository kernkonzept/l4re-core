
/*
 * Copyright (C) 2014-15 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <sysdep.h>

#ifdef DONT_USE_PREALLOC
#define PREWRITE(A,B)	prefetchw [(A),(B)]
#else
#define PREWRITE(A,B)	prealloc [(A),(B)]
#endif

ENTRY(memset)
	prefetchw [r0]		; Prefetch the write location
	mov.f	0, r2
;;; if size is zero
	jz.d	[blink]
	mov	r3, r0		; don't clobber ret val

;;; if length < 8
	brls.d.nt	r2, 8, .Lsmallchunk
	mov.f	lp_count,r2

	and.f	r4, r0, 0x03
	rsub	lp_count, r4, 4
	lpnz	@.Laligndestination
	;; LOOP BEGIN
	stb.ab	r1, [r3,1]
	sub	r2, r2, 1
.Laligndestination:

;;; Destination is aligned
	and	r1, r1, 0xFF
	asl	r4, r1, 8
	or	r4, r4, r1
	asl	r5, r4, 16
	or	r5, r5, r4
	mov	r4, r5

	sub3	lp_count, r2, 8
	cmp     r2, 64
	bmsk.hi	r2, r2, 5
	mov.ls	lp_count, 0
	add3.hi	r2, r2, 8

;;; Convert len to Dwords, unfold x8
	lsr.f	lp_count, lp_count, 6
	lpnz	@.Lset64bytes
	;; LOOP START
	PREWRITE(r3, 64)	;Prefetch the next write location
#if defined(__LL64__) || defined(__ARC_LL64__)
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
#else
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
#endif
.Lset64bytes:

	lsr.f	lp_count, r2, 5 ;Last remaining  max 124 bytes
	lpnz	.Lset32bytes
	;; LOOP START
	prefetchw [r3, 32]	;Prefetch the next write location
#if defined(__LL64__) || defined(__ARC_LL64__)
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
#else
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
#endif
.Lset32bytes:

	and.f	lp_count, r2, 0x1F ;Last remaining 31 bytes
.Lsmallchunk:
	lpnz	.Lcopy3bytes
	;; LOOP START
	stb.ab	r1, [r3, 1]
.Lcopy3bytes:

	j	[blink]

END(memset)
libc_hidden_def(memset)
