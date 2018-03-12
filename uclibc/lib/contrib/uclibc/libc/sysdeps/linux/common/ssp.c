/*
 * Distributed under the terms of the GNU Lesser General Public License
 * $Header: $
 *
 * This is a modified version of Hiroaki Etoh's stack smashing routines
 * implemented for glibc.
 *
 * The following people have contributed input to this code.
 * Ned Ludd - <solar[@]gentoo.org>
 * Alexander Gabert - <pappy[@]gentoo.org>
 * The PaX Team - <pageexec[@]freemail.hu>
 * Peter S. Mazinger - <ps.m[@]gmx.net>
 * Yoann Vandoorselaere - <yoann[@]prelude-ids.org>
 * Robert Connolly - <robert[@]linuxfromscratch.org>
 * Cory Visi <cory[@]visi.name>
 * Mike Frysinger <vapier[@]gentoo.org>
 */

#if defined __SSP__ || defined __SSP_ALL__
#error "file must not be compiled with stack protection enabled on it. Use -fno-stack-protector"
#endif

#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifdef __UCLIBC_HAS_SYSLOG__
#include <sys/syslog.h>
#endif

#ifdef __PROPOLICE_BLOCK_SEGV__
# define SSP_SIGTYPE SIGSEGV
#else
# define SSP_SIGTYPE SIGABRT
#endif

static void do_write(const char *msg)
{
	/* could use inlined syscall here to be sure ... */
	return (void) write(STDERR_FILENO, msg, strlen(msg));
}

static void __cold do_msg(const char *msg1, const char *msg2, const char *msg3)
{
	do_write(msg1);
	do_write(msg2);
	do_write(msg3);
	do_write("\n");
#ifdef __UCLIBC_HAS_SYSLOG__
	syslog(LOG_INFO, "%s%s%s()", msg1, msg2, msg3);
#endif
}

static void __cold attribute_noreturn
#ifdef __UCLIBC_HAS_SSP_COMPAT__
ssp_handler(char func[])
#else
ssp_handler(void)
#endif
{
	pid_t pid;
	static const char msg_ssd[] = "*** stack smashing detected ***: ";
	static const char msg_terminated[] = " terminated";
#ifdef __UCLIBC_HAS_SSP_COMPAT__
	static const char msg_ssa[] = ": stack smashing attack in function ";
#endif

#ifdef __DODEBUG__
	struct sigaction sa;
	sigset_t mask;

	__sigfillset(&mask);
	__sigdelset(&mask, SSP_SIGTYPE);	/* Block all signal handlers */
	sigprocmask(SIG_BLOCK, &mask, NULL);	/* except SSP_SIGTYPE */
#endif

#ifdef __UCLIBC_HAS_SSP_COMPAT__
	if (func != NULL)
		do_msg(__uclibc_progname, msg_ssa, func);
	else
#endif
		do_msg(msg_ssd, __uclibc_progname, msg_terminated);

	pid = getpid();
#ifdef __DODEBUG__
	/* Make the default handler associated with the signal handler */
	memset(&sa, 0, sizeof(sa));
	__sigfillset(&sa.sa_mask);	/* Block all signals */
	if (SIG_DFL) /* if it's constant zero, it's already done */
		sa.sa_handler = SIG_DFL;
	if (sigaction(SSP_SIGTYPE, &sa, NULL) == 0)
		(void)kill(pid, SSP_SIGTYPE);
#endif
	(void)kill(pid, SIGKILL);
	/* The loop is added only to keep gcc happy. */
	while(1)
		_exit(127);
}

#ifdef __UCLIBC_HAS_SSP_COMPAT__
void __stack_smash_handler(char func[], int damaged) attribute_noreturn __cold;
void __stack_smash_handler(char func[], int damaged attribute_unused)
{
	ssp_handler(func);
}

void __stack_chk_fail(void)
{
	ssp_handler(NULL);
}
#else
strong_alias(ssp_handler,__stack_chk_fail)
#endif

#ifdef __UCLIBC_HAS_FORTIFY__
/* should be redone when activated to use common code above.
 * for now, it works without debugging support */
void __chk_fail(void)
{
	static const char msg_fail[] = "*** buffer overflow detected ***: ";
	static const char msg_terminated[] = " terminated";
	pid_t pid;

	do_msg(msg_fail, __uclibc_progname, msg_terminated);

	pid = getpid();
	(void)kill(pid, SIGKILL);
	/* The loop is added only to keep gcc happy. */
	while(1)
		_exit(127);
}
libc_hidden_def(__chk_fail)
#endif
