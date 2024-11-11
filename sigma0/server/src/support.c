/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/sys/ipc.h>

void _exit(int) __attribute__((noreturn));

void _exit(int x)
{
  (void)x;
  for (;;)
    l4_ipc_sleep(L4_IPC_NEVER);
}
