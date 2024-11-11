/**
 * \file
 * \brief Event to ASCII key mapping
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4UTIL__KEYMAP_H__
#define __L4UTIL__KEYMAP_H__

#include <l4/sys/compiler.h>

__BEGIN_DECLS

int l4util_map_event_to_keymap(unsigned value, unsigned shift);

__END_DECLS


#endif /* __L4UTIL__KEYMAP_H__ */
