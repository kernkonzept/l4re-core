/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
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
