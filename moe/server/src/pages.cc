/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "pages.h"

l4_addr_t Moe::Pages::base_addr;
l4_addr_t Moe::Pages::max_addr;
l4_uint32_t *Moe::Pages::pages;

