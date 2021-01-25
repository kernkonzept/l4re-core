/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "boot_fs.h"
#include "loader_elf.h"
#include "debug.h"

#include <cstring>
#include <l4/bid_config.h>
#include <l4/libloader/elf>
#include <l4/sys/debugger.h>


bool
Elf_loader::launch(App_task *t, cxx::String const &prog,
                   cxx::String const &args)
{
  Dbg ldr(Dbg::Loader, "ldr");
  Moe_x_app_model am(t, prog, args);
#if defined(CONFIG_MMU) || defined(CONFIG_BID_PIE)
  Ldr::Elf_loader<Moe_x_app_model, Dbg>::launch(&am, "rom/l4re", ldr);
  l4_debugger_add_image_info(am._task->task_cap().cap(), 0, "l4re");
#else
  // We're running position dependent code without an MMU. The l4re_kernel can
  // thus be loaded only once. There's no point in using it.
  Ldr::Elf_loader<Moe_x_app_model, Dbg>::launch(&am, prog.start(), ldr);
  l4_debugger_add_image_info(am._task->task_cap().cap(), 0, am.argv.a0);
#endif
  return true;
}

bool
Elf_loader::check_file_type(Moe::Dataspace const *file) const
{
  char const *data = file->address(0).adr<char const *>();
  return memcmp(data, "\177ELF", 4) == 0;
}
