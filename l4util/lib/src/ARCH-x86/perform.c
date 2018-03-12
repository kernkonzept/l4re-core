/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/**
 * \brief   lib for performance mesaurement counters. 
 */

#include <l4/sys/types.h>

#define CONFIG_PERFORM_ONLY_PROTOTYPES
#include <l4/util/perform.h>

typedef struct
{
  l4_uint32_t index;
  const char *string;
} event_entry;

static const event_entry event_array[]=
{
#include "pmc_events.h"
};

const char*
strp6pmc_event(l4_uint32_t event)
{
  unsigned i;

  for (i=0; i<sizeof(event_array)/sizeof(event_entry); i++)
    if (event_array[i].index == event)
      return event_array[i].string;

  return "unkown";
}
