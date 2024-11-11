/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "page_alloc.h"

Page_alloc_base::Alloc Page_alloc_base::_alloc;
unsigned long Page_alloc_base::_total;

static char page_alloc_scratch_mem[L4_PAGESIZE] __attribute__((aligned(L4_PAGESIZE)));

void Page_alloc_base::init()
{ free(page_alloc_scratch_mem); }
