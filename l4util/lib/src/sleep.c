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

L4_CV void l4_sleep(l4_uint32_t ms)
{
  l4_utcb_t *u = l4_utcb();
  l4_msgtag_t tag = l4_ipc_sleep_ms(ms);
  if (l4_ipc_error(tag, u) != L4_IPC_RETIMEOUT)
    printf("l4_sleep(): IPC error %02x\n", l4_ipc_error_code(u));
}


L4_CV void l4_usleep(l4_uint64_t us)
{
  l4_utcb_t *u = l4_utcb();
  l4_msgtag_t tag = l4_ipc_sleep_us(us);
  if (l4_ipc_error(tag, u) != L4_IPC_RETIMEOUT)
    printf("l4_sleep(): IPC error %02x\n", l4_ipc_error_code(u));
}
