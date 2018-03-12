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

#include <l4/re/c/video/view.h>

#include <l4/re/video/view>

using namespace L4Re::Video;

L4_CV int
l4re_video_view_refresh(l4re_video_view_t *view, int x, int y, int w,
                        int h) L4_NOTHROW
{
  View *v = reinterpret_cast<View *>(view);
  return v->refresh(x, y, w, h);
}

L4_CV int
l4re_video_view_get_info(l4re_video_view_t *view,
                         l4re_video_view_info_t *info) L4_NOTHROW
{
  View *v = reinterpret_cast<View *>(view);
  return v->info(reinterpret_cast<View::Info*>(info));
}

L4_CV int
l4re_video_view_set_info(l4re_video_view_t *view,
                         l4re_video_view_info_t *info) L4_NOTHROW
{
  View *v = reinterpret_cast<View *>(view);
  return v->set_info(*reinterpret_cast<View::Info*>(info));
}

L4_CV int
l4re_video_view_set_viewport(l4re_video_view_t *view, int x, int y, int w,
                             int h, unsigned long bofs) L4_NOTHROW
{
  View *v = reinterpret_cast<View *>(view);
  return v->set_viewport(x, y, w, h, bofs);
}

L4_CV int
l4re_video_view_stack(l4re_video_view_t *view, l4re_video_view_t *pivot,
                      int behind) L4_NOTHROW
{
  View *v = reinterpret_cast<View *>(view);
  return v->stack(pivot ? *reinterpret_cast<View*>(pivot) : View(), behind);
}

