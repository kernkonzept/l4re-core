/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
