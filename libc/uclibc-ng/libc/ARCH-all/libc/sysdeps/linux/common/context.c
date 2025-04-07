#define __UCLIBC_HAS_CONTEXT_FUNCS__
#include <ucontext.h>
#include <stdio.h>

int getcontext(ucontext_t *ucp)
{
  (void)ucp;
  printf("getcontext: unimplemented\n");
  return -1;
}

int setcontext(const ucontext_t *ucp)
{
  (void)ucp;
  printf("setcontext: unimplemented\n");
  return -1;
}

void makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...)
{
  (void)ucp;
  (void)func;
  (void)argc;
  printf("makecontext: unimplemented\n");
}

int swapcontext(ucontext_t *restrict oucp,
                const ucontext_t *restrict ucp)
{
  (void)oucp;
  (void)ucp;
  printf("swapcontext: unimplemented\n");
  return -1;
}
