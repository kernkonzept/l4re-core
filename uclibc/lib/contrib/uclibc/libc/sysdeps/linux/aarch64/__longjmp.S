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
	ldp	x29, x30, [x0, #JB_X29<<3]
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__ && ! defined BID_VARIANT_FLAG_NOFPU
	ldp	 d8,  d9, [x0, #JB_D8<<3]
	ldp	d10, d11, [x0, #JB_D10<<3]
	ldp	d12, d13, [x0, #JB_D12<<3]
	ldp	d14, d15, [x0, #JB_D14<<3]
#endif
	ldr	x5, [x0, #JB_SP<<3]
	mov	sp, x5
	cmp	x1, #0
	mov	x0, #1
	csel	x0, x1, x0, ne
	/* Use br instead of ret because ret is guaranteed to mispredict */
	br	x30

libc_hidden_def(__longjmp)
