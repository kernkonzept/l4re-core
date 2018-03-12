/**
 * \file
 * \brief    IDT related functions
 * \ingroup  irq
 *
 * \date     2003
 * \author   Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __L4UTIL_IDT_H
#define __L4UTIL_IDT_H

#include <l4/sys/l4int.h>
#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

/**
 * \defgroup l4util_idt Functions to manipulate the local IDT
 * \ingroup l4util_api
 */
/*@{*/

/** IDT entry.
 */
typedef struct
{
  l4_uint64_t       a, b;		/**< see Intel doc */
} __attribute__ ((packed)) l4util_idt_desc_t;

/** Header of an IDT table.
 */
typedef struct
{
  l4_uint16_t       limit;		/**< limit field (see Intel doc) */
  void              *base;		/**< idt base (see Intel doc) */
  l4util_idt_desc_t desc[0];
} __attribute__ ((packed)) l4util_idt_header_t;

/** Create an IDT entry.
 * \param idt      pointer to idt table header
 * \param nr       # of exception vector
 * \param handler  exception handler
 */
static inline void
l4util_idt_entry(l4util_idt_header_t *idt, int nr, void(*handler)(void))
{
  idt->desc[nr].a = (l4_uint64_t)handler & 0x0000ffff;
  idt->desc[nr].b = 0x0000ef00 | ((l4_uint64_t)handler & 0xffff0000);
}

/** Initializes an IDT.
 * \param idt      pointer to idt table header
 * \param entries  # of of exception entries to hold by the idt table
 */
static inline void
l4util_idt_init(l4util_idt_header_t *idt, int entries)
{
  int i;
  idt->limit = entries*8 - 1;
  idt->base  = &idt->desc;

  for (i=0; i<entries; i++)
    l4util_idt_entry(idt, i, 0);
}

/** Set IDT table for the current thread 
 * \param idt      pointer to idt table header
 */
static inline void
l4util_idt_load(l4util_idt_header_t *idt)
{
  asm volatile ("lidt (%%rax) \n\t" : : "a" (idt));
}
/*@}*/
EXTERN_C_END

#endif

