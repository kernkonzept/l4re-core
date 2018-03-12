/*
 * Copyright (C) 2014 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 *
 * This file incorporates work covered by the following copyright notice:
 */

/*****************************************************************************/
/*!
 * \file    l4sys/include/ARCH-mips/consts.h
 * \brief   Common L4 constants, mips version
 * \ingroup api_types_mips
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
/*****************************************************************************/
#ifndef _L4_SYS_CONSTS_H
#define _L4_SYS_CONSTS_H

#include <l4/bid_config.h>
#ifndef __ASSEMBLY__
/* L4 includes */
#include <l4/sys/l4int.h>
#endif /* __ASSEMBLY__ */

/**
 * Sizeof a page in log2
 * \ingroup api_types_common
 */
#if defined(CONFIG_PAGE_SIZE_16KB)
#define L4_PAGESHIFT		14 // 16K pages
#else
#define L4_PAGESHIFT		12 // 4K pages
#endif

/**
 * Sizeof a large page in log2
 * \ingroup api_types_common
 */
#define L4_SUPERPAGESHIFT	22

#ifndef __ASSEMBLY__
#include_next <l4/sys/consts.h>
#endif /* __ASSEMBLY__ */

#endif /* !_L4_SYS_CONSTS_H */
