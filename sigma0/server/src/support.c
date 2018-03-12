/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/ipc.h>

void __aeabi_unwind_cpp_pr0(void);
void __aeabi_unwind_cpp_pr1(void);
void __aeabi_unwind_cpp_pr0(void) {}
void __aeabi_unwind_cpp_pr1(void) {}

void _exit(int) __attribute__((noreturn));

void _exit(int x)
{
  (void)x;
  l4_ipc_sleep(L4_IPC_NEVER);
  while (1)
    ;
}
