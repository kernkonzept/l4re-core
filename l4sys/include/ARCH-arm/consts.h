/**
 * \file
 * \brief   Common L4 constants, arm version
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef _L4_SYS_CONSTS_H
#define _L4_SYS_CONSTS_H

/* L4 includes */
#include <l4/sys/l4int.h>
/**
 * \addtogroup l4_memory_api
 */
/**@{*/
/**
 * Size of a page, log2-based.
 */
#define L4_PAGESHIFT		12

/**
 * Size of a large page, log2-based.
 */
#define L4_SUPERPAGESHIFT	21

/**@}*/

#include_next <l4/sys/consts.h>

#endif /* !_L4_SYS_CONSTS_H */
