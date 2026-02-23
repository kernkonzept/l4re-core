/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "dataspace.h"

namespace Dataspace_util
{
  unsigned long copy(Moe::Dataspace *dst, L4Re::Dataspace::Offset dst_offs,
      Moe::Dataspace const*src, L4Re::Dataspace::Offset src_offs,
      L4Re::Dataspace::Size size);

};

