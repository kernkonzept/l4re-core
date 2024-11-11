/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*
 * Message handler/dispatcher
 */

#include "debug.h"
#include "dispatcher.h"
#include "globals.h"
#include "region.h"

#include <l4/sys/cxx/ipc_epiface>

static Dbg dbg(Dbg::Server, "svr");

l4_msgtag_t
Dispatcher::dispatch(l4_msgtag_t t, l4_umword_t obj, l4_utcb_t *utcb)
{
  dbg.printf("request: tag=0x%lx proto=%ld obj=0x%lx\n", t.raw, t.label(), obj);
  return L4::Ipc::Dispatch<Region_map>::f(Global::local_rm.get(), t, obj, utcb);
}
