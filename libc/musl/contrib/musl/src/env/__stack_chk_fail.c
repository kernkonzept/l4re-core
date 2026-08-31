#include <string.h>
#include <stdint.h>
#ifndef NOT_FOR_L4
#include <unistd.h>
#endif
#include "pthread_impl.h"

uintptr_t __stack_chk_guard;

void __init_ssp(void *entropy)
{
	if (entropy) memcpy(&__stack_chk_guard, entropy, sizeof(uintptr_t));
	else __stack_chk_guard = (uintptr_t)&__stack_chk_guard * 1103515245;

#if UINTPTR_MAX >= 0xffffffffffffffff
	/* Sacrifice 8 bits of entropy on 64bit to prevent leaking/
	 * overwriting the canary via string-manipulation functions.
	 * The NULL byte is on the second byte so that off-by-ones can
	 * still be detected. Endianness is taken care of
	 * automatically. */
	((char *)&__stack_chk_guard)[1] = 0;
#endif

#ifdef NOT_FOR_L4
	__pthread_self()->canary = __stack_chk_guard;
#elif !defined(L4_MINIMAL_LIBC)
	/* On L4Re the TCB is the libpthread thread descriptor. The minimal libc
	 * has no thread pointer at all, hence only the global guard is set up
	 * there. */
	__pthread_descr_libc_data(__pthread_thread_self())->canary
		= __stack_chk_guard;
#endif
}

void __stack_chk_fail(void)
{
#if !defined(NOT_FOR_L4) && !defined(L4_MINIMAL_LIBC)
	/* Report what happened before dying. Deliberately no stdio here: the
	 * stack is already corrupted and stdio would take locks and consume
	 * even more stack. The minimal libc has no write() at all. */
	static const char msg1[] = "stack smashing detected: ";
	static const char msg2[] = " terminated\n";
	const char *pn = __progname ? __progname : "<unknown>";

	write(STDERR_FILENO, msg1, sizeof(msg1) - 1);
	write(STDERR_FILENO, pn, strlen(pn));
	write(STDERR_FILENO, msg2, sizeof(msg2) - 1);
#endif
	a_crash();
}

hidden void __stack_chk_fail_local(void);

weak_alias(__stack_chk_fail, __stack_chk_fail_local);
