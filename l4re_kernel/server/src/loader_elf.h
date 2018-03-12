/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/libloader/elf>
#include "loader.h"
#include "debug.h"

class Elf_loader
: public Loader,
  public Ldr::Elf_loader<L4Re_x_app_model, Dbg>
{
public:
  bool launch(L4::Cap<L4Re::Dataspace> bin, L4::Cap<L4Re::Rm>);
};

