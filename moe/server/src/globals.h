/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/kip.h>

#include "obj_reg.h"

namespace Moe { class Dataspace; };

extern L4::Cap<void> root_name_space_obj;
extern Moe::Dataspace *kip_ds;
extern "C" l4_kernel_info_t *_current_kip;
inline l4_kernel_info_t const *kip() { return _current_kip; }

extern Object_pool object_pool;
extern char const *const PROG;
extern char log_buffer[1024];

namespace Moe { namespace Virt_limit {
  extern l4_addr_t start;
  extern l4_addr_t end;
}}

namespace Moe {
  extern unsigned l4re_dbg;
  extern unsigned ldr_flags;
}

enum
{
  Sigma0_cap     = L4_BASE_PAGER_CAP,
};
