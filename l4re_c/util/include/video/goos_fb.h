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
#pragma once

#include <l4/sys/compiler.h>
#include <l4/re/c/video/goos.h>
#include <l4/sys/err.h>
#include <l4/re/c/dataspace.h>

__BEGIN_DECLS

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

__END_DECLS
