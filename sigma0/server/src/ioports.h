/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef SIGMA0_IO_PORTS_H__
#define SIGMA0_IO_PORTS_H__

#include <l4/sys/kip.h>
#include "memmap_internal.h"

void init_io_ports(l4_kernel_info_t *info);
void handle_io_page_fault(l4_umword_t t, l4_utcb_t *utcb, Answer *a);

void dump_io_ports();

#endif /* ! SIGMA0_IO_PORTS_H__ */
