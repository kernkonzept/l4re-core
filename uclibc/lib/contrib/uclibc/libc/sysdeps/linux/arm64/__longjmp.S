#include <features.h>
#include "jmpbuf-offsets.h"

.global __longjmp
.type __longjmp,%function

__longjmp:
	ldp	x19, x20, [x0, #JB_X19<<3]
	ldp	x21, x22, [x0, #JB_X21<<3]
	ldp	x23, x24, [x0, #JB_X23<<3]
	ldp	x25, x26, [x0, #JB_X25<<3]
	ldp	x27, x28, [x0, #JB_X27<<3]
#ifdef PTR_DEMANGLE
	ldp	x29,  x4, [x0, #JB_X29<<3]
	PTR_DEMANGLE (x30, x4, x3, x2)
#else
	ldp	x29, x30, [x0, #JB_X29<<3]
#endif
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
	ldp	 d8,  d9, [x0, #JB_D8<<3]
	ldp	d10, d11, [x0, #JB_D10<<3]
	ldp	d12, d13, [x0, #JB_D12<<3]
	ldp	d14, d15, [x0, #JB_D14<<3]
#endif
#ifdef PTR_DEMANGLE
	ldr	x4, [x0, #JB_SP<<3]
	PTR_DEMANGLE (x5, x4, x3, x2)
#else
	ldr	x5, [x0, #JB_SP<<3]
#endif
	mov	sp, x5
	cmp	x1, #0
	mov	x0, #1
	csel	x0, x1, x0, ne
	/* Use br instead of ret because ret is guaranteed to mispredict */
	br	x30

libc_hidden_def(__longjmp)
