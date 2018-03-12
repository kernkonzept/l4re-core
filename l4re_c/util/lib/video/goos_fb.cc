/**
 * \file
 * \brief Framebuffer utility functionality.
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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

#include <l4/re/util/video/goos_fb>
#include <l4/re/c/util/video/goos_fb.h>
#include <l4/cxx/exceptions>
#include <cstddef>

using L4Re::Util::Video::Goos_fb;
using L4Re::Video::Goos;
using L4Re::Video::View;

static inline Goos_fb *gcast(l4re_util_video_goos_fb_t *goosfb)
{
  (void)sizeof(char[sizeof(goosfb->_obj_buf) - sizeof(Goos_fb)]);
  return (Goos_fb *)goosfb->_obj_buf;
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
  try
    {
      init_goosfb(goosfb);
      gcast(goosfb)->setup(name);
    }
  catch (L4::Runtime_error &e) { return e.err_no(); }
  catch (...)                  { return -L4_EINVAL; }
  return 0;
}

L4_CV int
l4re_util_video_goos_fb_view_info(l4re_util_video_goos_fb_t *goosfb,
                                  l4re_video_view_info_t *info) L4_NOTHROW
{
  return gcast(goosfb)->view_info((L4Re::Video::View::Info *)info);
}

L4_CV void
l4re_util_video_goos_fb_destroy(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW
{
  gcast(goosfb)->~Goos_fb();
}

L4_CV void *
l4re_util_video_goos_fb_attach_buffer(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW
{
  try
    {
      return gcast(goosfb)->attach_buffer();
    }
  catch (...)
    {
      return 0;
    }
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
