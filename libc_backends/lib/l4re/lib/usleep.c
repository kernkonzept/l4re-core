/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/util.h>
#include <unistd.h>

int usleep(useconds_t usec)
{
  l4_usleep(usec);
  return 0;
}

