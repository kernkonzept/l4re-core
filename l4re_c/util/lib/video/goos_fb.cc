/**
 * \file
 * \brief Framebuffer utility functionality.
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/util/video/goos_fb>
#include <l4/re/c/util/video/goos_fb.h>
#include <cstddef>

using L4Re::Util::Video::Goos_fb;
using L4Re::Video::Goos;
using L4Re::Video::View;

static inline Goos_fb *gcast(l4re_util_video_goos_fb_t *goosfb)
{
  static_assert(sizeof(goosfb->_obj_buf) >= sizeof(Goos_fb),
                "buffer must be large enough to hold a Goos_fb");
  return reinterpret_cast<Goos_fb *>(goosfb->_obj_buf);
}

inline void *operator new(size_t, void *addr)
{ return addr; }

static inline void init_goosfb(l4re_util_video_goos_fb_t *goosfb)
{
  new (goosfb->_obj_buf) Goos_fb();
}

L4_CV int
l4re_util_video_goos_fb_setup_name(l4re_util_video_goos_fb_t *goosfb,
                                   char const *name) L4_NOTHROW
{
  static_assert(   offsetof(View::Info, pixel_info)
                == offsetof(l4re_video_view_info_t, pixel_info),
                "Structure alignment mismatch");
  static_assert(   offsetof(View::Info, buffer_index)
                == offsetof(l4re_video_view_info_t, buffer_index),
                "Structure alignment mismatch");
  static_assert(   offsetof(Goos::Info, pixel_info)
                == offsetof(l4re_video_goos_info_t, pixel_info),
                "Structure alignment mismatch");

  init_goosfb(goosfb);
  return gcast(goosfb)->init(name);
}

L4_CV int
l4re_util_video_goos_fb_view_info(l4re_util_video_goos_fb_t *goosfb,
                                  l4re_video_view_info_t *info) L4_NOTHROW
{
  return gcast(goosfb)
           ->view_info(reinterpret_cast<L4Re::Video::View::Info *>(info));
}

L4_CV void
l4re_util_video_goos_fb_destroy(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW
{
  gcast(goosfb)->~Goos_fb();
}

L4_CV void *
l4re_util_video_goos_fb_attach_buffer(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW
{
  return gcast(goosfb)->attach_buffer();
}

L4_CV int
l4re_util_video_goos_fb_refresh(l4re_util_video_goos_fb_t *goosfb,
                                int x, int y, int w, int h) L4_NOTHROW
{
  return gcast(goosfb)->refresh(x, y, w, h);
}

L4_CV l4re_ds_t
l4re_util_video_goos_fb_buffer(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW
{
  return gcast(goosfb)->buffer().cap();
}

L4_CV l4_cap_idx_t
l4re_util_video_goos_fb_goos(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW
{
  return gcast(goosfb)->goos().cap();
}
