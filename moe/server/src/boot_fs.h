/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/string>

namespace Moe {
class Dataspace;

class Boot_fs
{
public:
  static void init_stage1();
  static void init_stage2();

  static Moe::Dataspace *open_file(cxx::String const &name);
};

}

 
