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
#pragma once

#include <l4/sys/compiler.h>
#include <l4/re/c/video/goos.h>
#include <l4/sys/err.h>
#include <l4/re/c/dataspace.h>

EXTERN_C_BEGIN

typedef struct
{
  unsigned long _obj_buf[6];
} l4re_util_video_goos_fb_t;

L4_CV int
l4re_util_video_goos_fb_setup_name(l4re_util_video_goos_fb_t *goosfb,
                                   char const *name) L4_NOTHROW;

L4_CV void
l4re_util_video_goos_fb_destroy(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW;

L4_CV int
l4re_util_video_goos_fb_view_info(l4re_util_video_goos_fb_t *goosfb,
                                  l4re_video_view_info_t *info) L4_NOTHROW;

L4_CV void *
l4re_util_video_goos_fb_attach_buffer(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW;

L4_CV int
l4re_util_video_goos_fb_refresh(l4re_util_video_goos_fb_t *goosfb,
                                int x, int y, int w, int h) L4_NOTHROW;

L4_CV l4re_ds_t
l4re_util_video_goos_fb_buffer(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW;

L4_CV l4_cap_idx_t
l4re_util_video_goos_fb_goos(l4re_util_video_goos_fb_t *goosfb) L4_NOTHROW;

EXTERN_C_END
