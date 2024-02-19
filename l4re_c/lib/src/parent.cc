/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Marcus Haehnel <marcus.haehnel@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/parent>

#include <l4/re/c/parent.h>
#include <l4/sys/types.h>

L4_CV long L4_EXPORT
l4re_parent_signal(l4_cap_idx_t parent, unsigned long sig, unsigned long val)
{
  return L4::Cap<L4Re::Parent>(parent)->signal(sig, val);
}
