#ifndef __L4UTIL_SPARC_IRQ_H
#define __L4UTIL_SPARC_IRQ_H

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

#endif
