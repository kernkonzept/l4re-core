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

#include <l4/re/c/video/goos.h>

#include <l4/re/video/goos>
#include <l4/re/video/view>
#include <l4/re/dataspace>
#include <l4/sys/err.h>

#include <cassert>
#include <stddef.h>

using L4Re::Video::Goos;

L4_CV int
l4re_video_goos_info(l4re_video_goos_t goos,
                     l4re_video_goos_info_t *ginfo) L4_NOTHROW
{
  static_assert(   offsetof(Goos::Info, pixel_info)
                == offsetof(__typeof(*ginfo), pixel_info),
                "Structure alignment mismatch");
  L4::Cap<Goos> g(goos);
  return g->info((Goos::Info *)ginfo);
}
