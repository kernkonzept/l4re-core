/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef INIT_H
#define INIT_H

#include <l4/sys/compiler.h>

#define INIT_SECTION __attribute__((section (".init")))

#ifdef __cplusplus
extern "C" 
#endif
void init(l4_kernel_info_t *info) L4_NORETURN;

#endif
