/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "loader.h"
#include "debug.h"

#include <l4/libloader/elf>

class Elf_loader
: public Loader,
  public Ldr::Elf_loader<Moe_x_app_model, Dbg>

{
public:
  bool check_file_type(Moe::Dataspace const *file) const override;
  bool launch(App_task *, cxx::String const &, cxx::String const &) override;
};


