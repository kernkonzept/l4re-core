/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "ioports.h"
#include "mem_man.h"

#include <l4/sys/types.h>
#include <l4/sys/ipc.h>

static Mem_man io_ports;

void init_io_ports(l4_kernel_info_t * /*info*/)
{
}

void handle_io_page_fault(l4_umword_t /*t*/, l4_utcb_t * /*utcb*/, Answer *a)
{
  a->error(L4_EINVAL);
}

void dump_io_ports()
{}

