#include <sysdep.h>

        nop
        nop

ENTRY_NOCFI (__rt_sigreturn_stub)
        mov     __NR_rt_sigreturn, %g1
        ta      0x6d
END_NOCFI (__rt_sigreturn_stub)

