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

using namespace L4Re::Video;

L4_CV int
l4re_video_goos_create_buffer(l4re_video_goos_t goos, unsigned long size,
                              l4_cap_idx_t buffer) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  return g->create_buffer(size, L4::Cap<L4Re::Dataspace>(buffer));
}

L4_CV int
l4re_video_goos_refresh(l4re_video_goos_t goos, int x, int y, int w,
                        int h) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  return g->refresh(x, y, w, h);
}


L4_CV int
l4re_video_goos_delete_buffer(l4re_video_goos_t goos, unsigned idx) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  return g->delete_buffer(idx);
}

L4_CV int
l4re_video_goos_get_static_buffer(l4re_video_goos_t goos, unsigned idx,
                                  l4_cap_idx_t buffer) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  return g->get_static_buffer(idx, L4::Cap<L4Re::Dataspace>(buffer));
}

L4_CV int
l4re_video_goos_create_view(l4re_video_goos_t goos,
                            l4re_video_view_t *view) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  return g->create_view(reinterpret_cast<View*>(view));
}

L4_CV int
l4re_video_goos_delete_view(l4re_video_goos_t goos,
                            l4re_video_view_t *view) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  return g->delete_view(*reinterpret_cast<View*>(view));
}

L4_CV int
l4re_video_goos_get_view(l4re_video_goos_t goos, unsigned idx,
                         l4re_video_view_t *view) L4_NOTHROW
{
  L4::Cap<Goos> g(goos);
  *reinterpret_cast<View*>(view) = g->view(idx);
  return 0;
}

