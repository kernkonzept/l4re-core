/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/sys/kdebug.h>

long int
l4_atomic_add(volatile long int* mem, long int offset)
{
  long int ret = 0;
  asm volatile ( " 1:                            \n"
                 "  lwarx  %%r12, 0, %[ptr]      \n" //reserve
                 "  add    %[ret], %%r12, %[val] \n" //add
                 "  stwcx. %[ret], 0, %[ptr]     \n" //store if still reserved
                 "  bne-   1b                    \n" //repeat if store failed
                 : [ret]"=r" (ret),
                   [ptr]"=r" (mem),
                   [val]"=r" (offset)
                 : "0" (ret),
                   "1" (mem),
                   "2" (offset)
                 : "r12", "memory"
               );
  return ret;
}

long int
l4_atomic_cmpxchg(volatile long int* mem, long int oldval, long int newval)
{
  long int ret = 0;
  asm volatile ( " 1:                            \n"
                 "  lwarx  %%r12, 0, %[ptr]      \n"
                 "  cmpw   %[oldval], %%r12      \n"
                 "  bne-   2f                    \n"
                 "  stwcx. %[newval], 0,%[ptr]   \n"
                 "  bne-   1b                    \n"
                 " 2:                            \n"
                 "  mr     %[ret], %%r12         \n"
                 : [ret] "=r"(ret),
                   [ptr] "=r"(mem),
                   [oldval] "=r"(oldval),
                   [newval] "=r"(newval)
                 : "0" (ret),
                   "1" (mem),
                   "2" (oldval),
                   "3" (newval)
                 : "memory", "r12"
               );
  return (oldval == ret);
}

long int
l4_atomic_xchg(volatile long int* mem, long int newval)
{
  // someone not speaking ppc has added this...
  unsigned long r = *mem;
  outstring("l4_atomic_xchg is not atomic!\n");
  *mem = newval;
  return r;
}
