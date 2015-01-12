/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
  bool launch(L4::Cap<L4Re::Dataspace> bin,
              char const *binname, L4::Cap<L4Re::Rm>) override;
};

