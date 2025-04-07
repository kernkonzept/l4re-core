#include <sysdep.h>

	nop
	nop

ENTRY_NOCFI (__rt_sigreturn_stub)
	mov	__NR_rt_sigreturn, %g1
	ta	0x10
END_NOCFI (__rt_sigreturn_stub)

ENTRY_NOCFI (__sigreturn_stub)
	mov	__NR_sigreturn, %g1
	ta	0x10
END_NOCFI (__sigreturn_stub)
