/**
 * \file
 * \brief   Linkage
 * \ingroup l4sys_defines
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4__SYS__ARCH_ARM__LINKAGE_H__
#define __L4__SYS__ARCH_ARM__LINKAGE_H__

#ifdef __ASSEMBLY__
#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  .p2align(2); \
  name:
#endif
#endif

#define L4_FASTCALL(x)	x
#define l4_fastcall

/**
 * Define calling convention.
 * \ingroup l4sys_defines
 * \hideinitializer
 */
#define L4_CV

#ifdef __PIC__
# define L4_LONG_CALL
#else
# define L4_LONG_CALL __attribute__((long_call))
#endif

#endif /* ! __L4__SYS__ARCH_ARM__LINKAGE_H__ */
