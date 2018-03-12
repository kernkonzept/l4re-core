#include <l4/sys/ipc.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// taken from uclibc
#if defined __SSP__ || defined __SSP_ALL__
# error "file must not be compiled with stack protection enabled on it. Use -fno-stack-protector"
#endif

#ifdef UCLIBC_LDSO
# define PROGNAME UCLIBC_LDSO
#else
# define PROGNAME __uclibc_progname
#endif

uintptr_t _dl_setup_stack_chk_guard(void);
uintptr_t _dl_setup_stack_chk_guard(void)
{
  return 0xFF0A0D00UL; //terminator canary
}

#ifndef UCLIBC_LDSO // do not build for ldso

inline static void terminate(void)
{
  l4_ipc_sleep(l4_timeout(L4_IPC_TIMEOUT_NEVER, l4_timeout_rel(2, 9)));
}

static void __cold ssp_write(int fd, const char *msg1, const char *msg2, const char *msg3)
{
  write(fd, msg1, strlen(msg1));
  write(fd, msg2, strlen(msg2));
  write(fd, msg3, strlen(msg3));
  write(fd, "()\n", 3);
}

void __attribute__((noreturn)) __stack_chk_fail(void);
void __attribute__((noreturn)) __stack_chk_fail(void)
{
  static const char msg1[] = "stack smashing detected: ";
  static const char msg3[] = " terminated";

  ssp_write(STDERR_FILENO, msg1, PROGNAME, msg3);

  /* The loop is added only to keep gcc happy. */
  while(1)
    terminate();
}

void __attribute__((noreturn)) __stack_smash_handler(char func[], int damaged __attribute__ ((unused)));
void __attribute__((noreturn)) __stack_smash_handler(char func[], int damaged)
{
  static const char message[] = ": stack smashing attack in function ";

  ssp_write(STDERR_FILENO, PROGNAME,  message, func);

  /* The loop is added only to keep gcc happy. */
  while(1)
    terminate();
}

void __attribute__((noreturn)) __chk_fail(void);
libc_hidden_proto(__chk_fail)
void __attribute__((noreturn)) __chk_fail(void)
{
  static const char msg1[] = "buffer overflow detected: ";
  static const char msg3[] = " terminated";

  ssp_write(STDERR_FILENO, msg1, PROGNAME, msg3);

  /* The loop is added only to keep gcc happy. */
  while(1)
    terminate();
}
libc_hidden_def(__chk_fail)

#endif // UCLIBC_LDSO
