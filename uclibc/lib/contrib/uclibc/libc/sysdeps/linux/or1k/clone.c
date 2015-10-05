/*
 * clone syscall for OpenRISC
 *
 *  Copyright (c) 2010     Jonas Bonn <jonas@southpole.se>
 *  Copyright (C) 2003     John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * OpenRISC port by Jonas Bonn <jonas@southpole.se>
 */

#include <errno.h>
#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>

/* The userland implementation is:
   int clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...)
   the kernel entry is:
   int clone (long flags, void *child_stack)
*/

int
clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...)
{
        int err;

	/* OK, here's the skinny on this one...
	 * OR1K GCC does weird things with varargs functions... the last
	 * parameter is NEVER passed on the stack -- i.e. arg, in this case.
	 * So we need to push at least 'arg' onto the child stack so that
	 * the new thread can find it.  Just to be totally safe, we'll
	 * push both 'fn' and 'arg'; that way we don't need to care what
	 * GCC does with parameters, whether they are passed in registers
	 * or on stack.
	 */

	/* Put 'fn' and 'arg' on child stack */
	__asm__ __volatile__ (
		"l.sw  -4(%0),%1;"
		"l.sw  -8(%0),%2;"
		:
		: "r" (child_stack), "r" (fn), "r" (arg)
		);

        /* Sanity check the arguments */
        err = -EINVAL;
        if (!fn)
                goto syscall_error;
        if (!child_stack)
                goto syscall_error;

	err = INLINE_SYSCALL(clone, 2, flags, child_stack);

	/* NB: from here you are in child thread or parent thread.
	 *
	 * Do not use any functions here that may write data _up_
	 * onto the stack because they will overwrite the child's
	 * thread descriptor... i.e. don't use printf
	 */

        if (err < 0)
                goto syscall_error;
        else if (err != 0) {
                return err;
	}

	/* NB: from here you exclusively in child thread */

	/* Grab 'fn' and 'arg' from child stack */
	__asm__ __volatile__ (
		"l.lwz  %0,-4(%2);"
		"l.lwz  %1,-8(%2);"
		: "=&r" (fn), "=r" (arg)
		: "r" (child_stack)
		: "0", "1"
		);

	_exit(fn(arg));

syscall_error:
        __set_errno (-err);
        return -1;
}
