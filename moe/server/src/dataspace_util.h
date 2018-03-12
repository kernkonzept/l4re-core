/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "dataspace.h"

namespace Dataspace_util
{
  unsigned long copy(Moe::Dataspace *dst, unsigned long dst_offs,
      Moe::Dataspace const*src, unsigned long src_offs, unsigned long size);

};

