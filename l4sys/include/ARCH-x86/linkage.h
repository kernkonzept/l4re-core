/**
 * \file
 * \brief   Linkage
 * \ingroup l4sys_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
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
#ifndef __L4__SYS__ARCH_X86__LINKAGE_H__
#define __L4__SYS__ARCH_X86__LINKAGE_H__

#ifdef __ASSEMBLY__

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  .p2align(2); \
  name:

#endif /* ! ENTRY */
#endif /* __ASSEMBLY__ */

#define L4_FASTCALL(x)	x __attribute__((regparm(3)))
#define l4_fastcall	__attribute__((regparm(3)))

/**
 * Define calling convention.
 * \ingroup l4sys_defines
 * \hideinitializer
 */
#define L4_CV		__attribute__((regparm(0)))

#endif /* ! __L4__SYS__ARCH_X86__LINKAGE_H__ */
