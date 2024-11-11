/*****************************************************************************/
/**
 * \file
 * \brief   Common L4 constants, amd64 version.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*****************************************************************************/
#ifndef __L4SYS__INCLUDE__ARCH_AMD64__CONSTS_H__
#define __L4SYS__INCLUDE__ARCH_AMD64__CONSTS_H__

/**
 * Size of a page, log2-based.
 * \ingroup l4_memory_api
 */
#define L4_PAGESHIFT		12

/**
 * Size of a large page, log2-based.
 * \ingroup l4_memory_api
 */
#define L4_SUPERPAGESHIFT	21

#include_next <l4/sys/consts.h>

#endif /* ! __L4SYS__INCLUDE__ARCH_AMD64__CONSTS_H__ */
