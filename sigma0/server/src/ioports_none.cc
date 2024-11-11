/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "ioports.h"
#include "mem_man.h"

#include <l4/sys/types.h>
#include <l4/sys/ipc.h>

void init_io_ports()
{
}

void handle_io_page_fault(l4_umword_t /*t*/, l4_utcb_t * /*utcb*/, Answer *a)
{
  a->error(L4_EINVAL);
}

void dump_io_ports()
{}
