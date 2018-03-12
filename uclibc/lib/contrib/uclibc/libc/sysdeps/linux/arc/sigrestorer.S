/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>
#include <sys/syscall.h>

/*
 * Provide a real sigreturn stub to avoid kernel synthesizing one
 * on user stack at runtime, which needs PTE permissions update
 * (hence TLB entry update) and costly cache line flushes for
 * code modification
 */

ENTRY(__default_rt_sa_restorer)
    mov r8, __NR_rt_sigreturn
    ARC_TRAP_INSN
END(__default_rt_sa_restorer)
libc_hidden_def(__default_rt_sa_restorer)
