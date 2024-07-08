/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file "COPYING" in the main directory of this archive for
 * more details.
 *
 * Copyright (C) 2025 Kalray Inc.
 * Author(s): Julian Vetter <jvetter@kalrayinc.com>
 */

#include <stdarg.h>
#include <ucontext.h>


/* Number of arguments that go in registers.	*/
#define NREG_ARGS 12

/* Take a context previously prepared via getcontext() and set to
	 call func() with the given int only args.	*/
void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
	extern void __startcontext (void);
	unsigned long *funcstack;
	va_list vl;
	unsigned long *regptr;
	unsigned int reg;

	/* Start at the top of stack.	*/
	funcstack = (unsigned long *) (ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
	funcstack -= argc < NREG_ARGS ? 0 : argc - NREG_ARGS;
	funcstack = (unsigned long *) (((uintptr_t) funcstack & -32L));

	ucp->uc_mcontext.sc_regs.r12 = (unsigned long) funcstack;
	/* Use $r20 and $r21 to pass some infos to __startcontext */
	ucp->uc_mcontext.sc_regs.r20 = (unsigned long) ucp->uc_link;
	ucp->uc_mcontext.sc_regs.r21 = (unsigned long) func;
	ucp->uc_mcontext.sc_regs.ra = (unsigned long) __startcontext;

	va_start (vl, argc);

	/* The first twelve arguments go into registers.	*/
	regptr = &(ucp->uc_mcontext.sc_regs.r0);

	for (reg = 0; (reg < argc) && (reg < NREG_ARGS); reg++)
		*regptr++ = va_arg (vl, unsigned long);

	/* And the remainder on the stack.	*/
	for (; reg < argc; reg++)
		*funcstack++ = va_arg (vl, unsigned long);

	va_end (vl);
}
weak_alias (__makecontext, makecontext)
