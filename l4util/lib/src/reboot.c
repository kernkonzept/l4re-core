/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/sys/kdebug.h>
#include <l4/util/reboot.h>

L4_CV void
l4util_reboot(void)
{
  enter_kdebug("*#^");          /* Always available */

  enter_kdebug("Exit failed!"); /* Should we loop here? */

  while (1)
    ;
}
