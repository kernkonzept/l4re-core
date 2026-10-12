/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "pages.h"

#ifdef CONFIG_MMU
Moe::Pages::Bank *Moe::Pages::banks;
unsigned Moe::Pages::num_banks;
#endif
