#ifndef _ASM_POWERPC_SIGCONTEXT_H
#define _ASM_POWERPC_SIGCONTEXT_H

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

struct pt_regs {
	unsigned long gpr[32];
	unsigned long nip;
	unsigned long msr;
	unsigned long orig_gpr3;	/* Used for restarting system calls */
	unsigned long ctr;
	unsigned long link;
	unsigned long xer;
	unsigned long ccr;
	unsigned long mq;		/* 601 only (not used at present) */
	unsigned long trap;		/* Reason for being here */
	/* N.B. for critical exceptions on 4xx, the dar and dsisr
	   fields are overloaded to hold srr0 and srr1. */
	unsigned long dar;		/* Fault registers */
	unsigned long dsisr;		/* on 4xx/Book-E used for ESR */
	unsigned long result;		/* Result of a system call */
};

struct sigcontext {
	unsigned long	_unused[4];
	int		signal;
#ifdef __powerpc64__
	int		_pad0;
#endif
	unsigned long	handler;
	unsigned long	oldmask;
	struct pt_regs	*regs;
};

#endif /* _ASM_POWERPC_SIGCONTEXT_H */
