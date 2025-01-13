/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/c/video/goos.h>

#include <l4/re/video/goos>
#include <l4/re/video/view>
#include <l4/re/dataspace>
#include <l4/sys/err.h>

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
  return g->info(reinterpret_cast<Goos::Info *>(ginfo));
}
