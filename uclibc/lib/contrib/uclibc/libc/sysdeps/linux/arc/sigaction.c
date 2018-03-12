/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/kernel_sigaction.h>

extern void __default_rt_sa_restorer(void);
//libc_hidden_proto(__default_rt_sa_restorer);

#define SA_RESTORER	0x04000000

/* If @act is not NULL, change the action for @sig to @act.
   If @oact is not NULL, put the old action for @sig in @oact.  */
int
__libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
	struct sigaction kact;

	/* !act means caller only wants to know @oact
	 * Hence only otherwise, do SA_RESTORER stuff
	 *
	 * For the normal/default cases (user not providing SA_RESTORER) use
	 * a real sigreturn stub to avoid kernel synthesizing one on user stack
	 * at runtime, which needs PTE permissions update (hence TLB entry
	 * update) and costly cache line flushes for code modification
	 */
	if (act && !(act->sa_flags & SA_RESTORER)) {
		memcpy(&kact, act, sizeof(kact));
		kact.sa_restorer = __default_rt_sa_restorer;
		kact.sa_flags |= SA_RESTORER;

		act = &kact;
	}

	return __syscall_rt_sigaction(sig, act, oact, sizeof(act->sa_mask));
}

#ifndef LIBC_SIGACTION
weak_alias(__libc_sigaction,sigaction)
libc_hidden_weak(sigaction)
#endif
