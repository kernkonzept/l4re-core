/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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

