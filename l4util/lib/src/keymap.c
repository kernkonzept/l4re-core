/*!
 * \file
 * \brief  convert keyboard value to ascii value
 *
 * \date   Dec 2007
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2007-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/util/keymap.h>

#ifdef USE_DE_KEYMAP
#include "keymap_de.h"
#else
#include "keymap_en.h"
#endif

int l4util_map_event_to_keymap(unsigned value, unsigned shift)
{
  if (value < 128 && shift < 2)
    return keymap[value][shift];

  return 0;
}
