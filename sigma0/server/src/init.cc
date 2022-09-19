/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
/* startup stuff */

/* it should be possible to throw away the text/data/bss of the object
   file resulting from this source -- so, we don't define here
   anything we could still use at a later time.  instead, globals are
   defined in globals.c */

#include <l4/bid_config.h>
#include <l4/sys/ipc.h>
#include <l4/sys/scheduler>
#include <l4/cxx/iostream>

#include "globals.h"
#include "mem_man.h"
#include "page_alloc.h"
#include "memmap.h"
#include "init.h"
#include "init_mem.h"
#include "ioports.h"
#include <l4/sys/debugger.h>

/* started as the L4 sigma0 task from crt0.S */

extern "C" void _init(void);

typedef void Ctor();
extern Ctor *__preinit_array_start[];
extern Ctor *__preinit_array_end [];
extern Ctor *__init_array_start [];
extern Ctor *__init_array_end [];

static void call_init_array(Ctor **start, Ctor **end)
{
  for (; start < end; ++start)
    if (*start)
      (*start)();
}

#ifdef CONFIG_BID_PIE
static inline unsigned long elf_machine_dynamic()
{
  extern const unsigned long _GLOBAL_OFFSET_TABLE_[] __attribute__((visibility ("hidden")));
  return _GLOBAL_OFFSET_TABLE_[0];
}

static inline unsigned long elf_machine_load_address()
{
  extern char _DYNAMIC[] __attribute__((visibility ("hidden")));
    return (unsigned long)&_DYNAMIC - elf_machine_dynamic ();
}
#else
static inline unsigned long elf_machine_load_address()
{ return 0; }
#endif

void
init(l4_kernel_info_t *info)
{
  call_init_array(__preinit_array_start, __preinit_array_end);
  _init();
  call_init_array(__init_array_start, __init_array_end);

  l4_info = info;

  L4::cout << PROG_NAME": Hello!\n";
  L4::cout << "  KIP @ " << info << '\n';

  l4_debugger_set_object_name(L4_BASE_TASK_CAP,    "sigma0");
  l4_debugger_set_object_name(L4_BASE_FACTORY_CAP, "root factory");
  l4_debugger_set_object_name(L4_BASE_THREAD_CAP,  "sigma0");
  l4_debugger_add_image_info(L4_BASE_TASK_CAP, elf_machine_load_address(), "sigma0");

  Page_alloc_base::init();

  init_memory(info);
  init_io_ports();

  L4::cout << "  allocated " << Page_alloc_base::total()/1024
           << "KB for maintenance structures\n";

  auto sched = L4::Cap<L4::Scheduler>(L4_BASE_SCHEDULER_CAP);
  l4_msgtag_t res = sched->run_thread(L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP),
                                      l4_sched_param(CONFIG_SIGMA0_PRIORITY));
  if (l4_error(res) < 0)
    L4::cout << PROG_NAME": could not set scheduling priority\n";

  if (debug_memory_maps)
    dump_all();

  /* now start the memory manager */
  pager();
}
