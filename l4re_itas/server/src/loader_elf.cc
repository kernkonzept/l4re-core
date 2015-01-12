/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "loader_elf.h"
#include "debug.h"

#include <l4/libloader/elf>

static Dbg ldr(Dbg::Loader, "ldr");


bool
Elf_loader::launch(L4::Cap<L4Re::Dataspace> bin,
                   char const *binname, L4::Cap<L4Re::Rm> rm)
{
  Ldr::Elf_loader<L4Re_x_app_model, Dbg>::launch(rm, nullptr,
                                                 bin, binname, ldr);
  return true;
}
