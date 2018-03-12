/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "globals.h"
#include <cstdio>

l4_kernel_info_t *_current_kip;
L4::Cap<void> root_name_space_obj;

static Cap_alloc _cap_allocator __attribute__((init_priority(1400)));
Object_pool __attribute__((init_priority(1401))) object_pool(&_cap_allocator);

char log_buffer[1024];
Moe::Dataspace *kip_ds;
extern char const *const PROG = "moe";
