/**
 * \file
 * \brief    some PIC and hardware interrupt related functions
 *
 * \date     2003
 * \author   Jork Loeser <jork.loeser@inf.tu-dresden.de>
 *           Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef __L4_IRQ_H__ 
#define __L4_IRQ_H__ 

#include <l4/sys/compiler.h>
#include <l4/util/port_io.h>

L4_BEGIN_DECLS

/**
 * \addtogroup l4util_cpu
 */
/**@{*/

/** Disable all interrupts
 */
static inline void
l4util_cli (void)
{
  __asm__ __volatile__ ("cli" : : : "memory");
}

/** Enable all interrupts
 */
static inline void
l4util_sti (void)
{
  __asm__ __volatile__ ("sti" : : : "memory");
}

/** Save the processor flags. Can be used to save and later restore the
 * interrupt flag 
 */
static inline void
l4util_flags_save (l4_umword_t *flags)
{
  __asm__ __volatile__ ("pushfl ; popl %0 " :"=g" (*flags) : :"memory");
}

/** Restore processor flags. Can be used to restore the interrupt flag
 */
static inline void
l4util_flags_restore (l4_umword_t *flags)
{
  __asm__ __volatile__ ("pushl %0 ; popfl" : :"g" (*flags) : "memory");
}
/**@}*/

L4_END_DECLS

#endif
