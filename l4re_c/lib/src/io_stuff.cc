/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include <l4/sys/types.h>
#include <l4/sys/kdebug.h>

#if 0
extern "C" int l4io_search_mem_region(l4_addr_t addr,
                           l4_addr_t *start, l4_size_t *len)
{
  enter_kdebug("l4io_search_mem_region");
  return 0;
}

extern "C" int l4io_request_region(l4_uint16_t start, l4_uint16_t len)
{
  enter_kdebug("l4io_request_region");
  return 0;
}

extern "C" int l4io_release_mem_region(l4_addr_t start, l4_size_t len)
{
  enter_kdebug("l4io_release_mem_region");
  return 0;
}

extern "C" l4_addr_t l4io_request_mem_region(l4_addr_t start, l4_size_t len,
                                  int flags)
{
  enter_kdebug("l4io_request_mem_region");
  return 0;
}
#endif
