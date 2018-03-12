/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Michael Hohmuth <hohmuth@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/*
 */

/*****************************************************************************
 * libl4util/src/sleep.c                                                     *
 * suspend thread                                                            *
 *****************************************************************************/

#include <stdio.h>
#include <l4/sys/ipc.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/kernel_object.h>
#include <l4/util/util.h>

L4_CV void l4_sleep(int ms)
{
  l4_timeout_t to;
  l4_msgtag_t tag;
  l4_utcb_t *u = l4_utcb();

  if (ms != -1)
    /* calculate timeout */
    to = l4_timeout(L4_IPC_TIMEOUT_NEVER,l4util_micros2l4to(ms * 1000));
  else
    to = L4_IPC_NEVER;

  tag = l4_ipc_receive(L4_INVALID_CAP, u, to);

  if (l4_ipc_error(tag, u) != L4_IPC_RETIMEOUT)
    printf("l4_sleep(): IPC error %02x\n", l4_ipc_error_code(u));
}


L4_CV void l4_usleep(int us)
{
  l4_msgtag_t tag;
  l4_timeout_t to;
  l4_utcb_t *u = l4_utcb();

  /* calculate timeout */
  to = l4_timeout(L4_IPC_TIMEOUT_NEVER, l4util_micros2l4to(us));

  tag = l4_ipc_sleep(to);

  if (l4_ipc_error(tag, u) != L4_IPC_RETIMEOUT)
    printf("l4_sleep(): IPC error %02x\n", l4_ipc_error_code(u));
}
