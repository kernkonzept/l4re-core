/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "pages.h"

#ifdef CONFIG_MMU
l4_addr_t Moe::Pages::base_addr;
l4_addr_t Moe::Pages::max_addr;
l4_uint32_t *Moe::Pages::pages;
#endif
