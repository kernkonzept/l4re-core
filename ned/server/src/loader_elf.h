/*
 * (c) 2008-2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once


#include "loader.h"
#include "debug.h"

#include <l4/libloader/elf>
#include <l4/sys/capability>
#include <l4/re/dataspace>

class App_task;



class Elf_loader
: public Loader,
  public Ldr::Elf_loader<Ldr_x_app_model, Dbg>
{
public:
  bool check_file_type(L4::Cap<L4Re::Dataspace> file) const;
  bool launch(App_task *, Ldr::Script *);
};


