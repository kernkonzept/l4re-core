/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

int dl_iterate_phdr(void *callback, void *data) __attribute__ (( __weak__));
int dl_iterate_phdr(void *callback, void *data)
{
  (void)callback;
  (void)data;
  return -1;
}
