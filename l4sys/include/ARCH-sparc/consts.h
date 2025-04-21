/*****************************************************************************/
/*!
 * \file
 * \brief   Common L4 constants, sparc version
 * \ingroup api_types_sparc
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

/* L4 includes */
#include <l4/sys/l4int.h>

/**
 * Sizeof a page in log2
 * \ingroup api_types_common
 */
#define L4_PAGESHIFT		12

/**
 * Sizeof a large page in log2
 * \ingroup api_types_common
 */
#define L4_SUPERPAGESHIFT	22

#include_next <l4/sys/consts.h>

#endif /* !_L4_SYS_CONSTS_H */
