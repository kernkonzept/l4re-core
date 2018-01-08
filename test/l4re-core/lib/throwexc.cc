/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Marius Melzer <marius.melzer@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include "throwexc.h"

void Throw_exc::throw_str_exc()
{
  throw "string exception";
}

void Throw_exc::throw_base_exc()
{
  throw Base_exc();
}

void Throw_exc::throw_derived_exc()
{
  throw Derived_exc();
}

namespace
{
  class Hidden_derived_exc : public Throw_exc::Base_exc
  {};
}

void Throw_exc::throw_hidden_derived_exc()
{
  throw Hidden_derived_exc();
}
