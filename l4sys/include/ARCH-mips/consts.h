/*
 * Copyright (C) 2014 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 *
 * This file incorporates work covered by the following copyright notice:
 */

/*****************************************************************************/
/*!
 * \file
 * \brief   Common L4 constants, mips version
 * \ingroup api_types_mips
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
