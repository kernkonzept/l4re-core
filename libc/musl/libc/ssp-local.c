/*
 * Local stack protector failure handler.
 *
 * On some architectures (e.g. x86-32) gcc calls __stack_chk_fail_local()
 * instead of __stack_chk_fail() to avoid the needless PIC pointer setup that
 * would be required at the call site. The symbol has hidden visibility and
 * can therefore not be provided by libc.so. Musl only defines it as a weak
 * alias in libc.a, which leaves shared libraries and dynamically linked
 * programs with an unresolved reference. They pick it up from
 * libc_nonshared.p.a instead.
 *
 * Must never be compiled with the stack protector enabled.
 */

void __attribute__((noreturn)) __stack_chk_fail(void);
void __attribute__((noreturn, visibility("hidden"))) __stack_chk_fail_local(void);

void __stack_chk_fail_local(void)
{
  __stack_chk_fail();
}
