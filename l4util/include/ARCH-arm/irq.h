/**
 * \file
 * \brief ARM specific implementation of irq functions
 *
 * Do not use.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4UTIL__ARCH_ARCH__IRQ_H__
#define __L4UTIL__ARCH_ARCH__IRQ_H__

#ifdef __GNUC__

#include <l4/sys/compiler.h>

L4_BEGIN_DECLS

L4_INLINE void l4util_cli (void);
L4_INLINE void l4util_sti (void);
L4_INLINE void l4util_flags_save(l4_umword_t *flags);
L4_INLINE void l4util_flags_restore(l4_umword_t *flags);

L4_INLINE
void
l4util_cli(void)
{
  extern void __do_not_use_l4util_cli(void);
  __do_not_use_l4util_cli();
}


L4_INLINE
void
l4util_sti(void)
{
  extern void __do_not_use_l4util_sti(void);
  __do_not_use_l4util_sti();
}


L4_INLINE
void
l4util_flags_save(l4_umword_t *flags)
{
  (void)flags;
  extern void __do_not_use_l4util_flags_save(void);
  __do_not_use_l4util_flags_save();
}

L4_INLINE
void
l4util_flags_restore(l4_umword_t *flags)
{
  (void)flags;
  extern void __do_not_use_l4util_flags_restore(void);
  __do_not_use_l4util_flags_restore();
}

L4_END_DECLS

#endif //__GNUC__

#endif /* ! __L4UTIL__ARCH_ARCH__IRQ_H__ */
