/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/util/l4mod.h>

#ifdef CONFIG_MOE_VESA_FB
void init_vesa_fb(l4util_l4mod_info *mbi);
#else
static inline void init_vesa_fb(l4util_l4mod_info *)
{}
#endif
