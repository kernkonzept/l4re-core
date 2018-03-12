/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "loader_elf.h"
#include "debug.h"

#include <l4/libloader/elf>

static Dbg ldr(Dbg::Loader, "ldr");


bool
Elf_loader::launch(L4::Cap<L4Re::Dataspace> bin, L4::Cap<L4Re::Rm> rm)
{
  Ldr::Elf_loader<L4Re_x_app_model, Dbg>::launch(rm, (void*)0, bin, ldr);
  return true;
}
