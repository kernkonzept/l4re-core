/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/cxx/string>

namespace Moe
{
void init_root_factory_config(cxx::String config, cxx::String brk);
void init_default_mem_cfg();
} // namespace Moe
