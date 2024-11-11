/*
 * (c) 2004-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/sys/ipc.h>
#include <l4/cxx/iostream>

extern "C"
void abort(void) __attribute((noreturn));

void abort(void)
{
  L4::cerr << "Aborted\n";
  for (;;)
    l4_ipc_sleep(L4_IPC_NEVER, l4sys_utcb());
}
