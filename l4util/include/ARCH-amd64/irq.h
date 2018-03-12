/**
 * \file
 * \brief    some PIC and hardware interrupt related functions
 * \ingroup  irq
 *
 * \date     2003
 * \author   Jork Loeser <jork.loeser@inf.tu-dresden.de>
 *           Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __L4_IRQ_H__ 
#define __L4_IRQ_H__ 

#include <l4/sys/compiler.h>
#include <l4/util/port_io.h>

EXTERN_C_BEGIN

/** Acknowledge IRQ at PIC in fully special nested mode.
 * \param irq  number of interrupt to acknowledge 
 */
L4_INLINE void
l4util_irq_acknowledge(unsigned int irq);

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
  __asm__ __volatile__ ("pushf ; popq %0 " :"=g" (*flags) : :"memory");
}

/** Restore processor flags. Can be used to restore the interrupt flag
 */
static inline void
l4util_flags_restore (l4_umword_t *flags)
{
  __asm__ __volatile__ ("pushq %0 ; popf" : :"g" (*flags) : "memory");
}

L4_INLINE void
l4util_irq_acknowledge(unsigned int irq)
{
  if (irq > 7)
    {
      l4util_out8(0x60+(irq & 7), 0xA0);
      l4util_out8(0x0B,0xA0);
      if (l4util_in8(0xA0) == 0)
	l4util_out8(0x60 + 2, 0x20);
    } 
  else
    l4util_out8(0x60+irq, 0x20);     /* acknowledge the irq */
};

EXTERN_C_END

#endif
