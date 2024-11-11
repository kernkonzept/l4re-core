/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef MEMMAP_H
#define MEMMAP_H

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/sys/kip.h>

#include "mem_man.h"
#include "globals.h"


extern Mem_man iomem;

extern l4_kernel_info_t *l4_info;

#ifdef __cplusplus
extern "C" {
#endif
void pager(void) L4_NORETURN;
void dump_all(void);

#ifdef __cplusplus
}
#endif


#endif
