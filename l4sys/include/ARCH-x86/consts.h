/*****************************************************************************/
/**
 * \file
 * \brief   Common L4 constants, x86 version
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
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
#ifndef __L4SYS__INCLUDE__ARCH_X86__CONSTS_H__
#define __L4SYS__INCLUDE__ARCH_X86__CONSTS_H__

/**
 * Size of a page log2-based
 * \ingroup l4_memory_api
 */
#define L4_PAGESHIFT		12

/**
 * Size of a large page log2-based
 * \ingroup l4_memory_api
 */
#define L4_SUPERPAGESHIFT	22

#include_next <l4/sys/consts.h>

#endif /* ! __L4SYS__INCLUDE__ARCH_X86__CONSTS_H__ */
