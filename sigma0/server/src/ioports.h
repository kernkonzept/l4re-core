/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef SIGMA0_IO_PORTS_H__
#define SIGMA0_IO_PORTS_H__

#include <l4/sys/kip.h>
#include "memmap_internal.h"

void init_io_ports();
void handle_io_page_fault(l4_umword_t t, l4_utcb_t *utcb, Answer *a);

void dump_io_ports();

#endif /* ! SIGMA0_IO_PORTS_H__ */
