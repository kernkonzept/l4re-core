/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 * Copyright (C) 2007 ARC International (UK) LTD
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

#define SMALL	7 /* Must be at least 6 to deal with alignment/loop issues.  */

ENTRY(memset)

	mov_s	r4,r0
	or	r12,r0,r2
	bmsk.f	r12,r12,1
	extb_s	r1,r1
	asl	r3,r1,8
	beq.d	.Laligned
	or_s	r1,r1,r3
	brls	r2,SMALL,.Ltiny
	add	r3,r2,r0
	stb	r1,[r3,-1]
	bclr_s	r3,r3,0
	stw	r1,[r3,-2]
	bmsk.f	r12,r0,1
	add_s	r2,r2,r12
	sub.ne	r2,r2,4
	stb.ab	r1,[r4,1]
	and	r4,r4,-2
	stw.ab	r1,[r4,2]
	and	r4,r4,-4
.Laligned:	; This code address should be aligned for speed.
	asl	r3,r1,16
	lsr.f	lp_count,r2,2
	or_s	r1,r1,r3
	lpne	.Loop_end
	st.ab	r1,[r4,4]
.Loop_end:
	j_s	[blink]


	.balign	4
.Ltiny:
	mov.f	lp_count,r2
	lpne	.Ltiny_end
	stb.ab	r1,[r4,1]
.Ltiny_end:
	j_s	[blink]
END(memset)
libc_hidden_def(memset)
