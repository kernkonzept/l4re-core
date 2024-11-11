/**
 * \file
 * \note The C interface of L4Re::Video does _NOT_ reflect the full C++
 *       interface on purpose. Use the C++ interface where possible.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>

/**
 * \brief Color component structure
 * \ingroup api_l4re_c_video
 */
typedef struct l4re_video_color_component_t
{
  unsigned char size;                     ///< Size in bits
  unsigned char shift;                    ///< offset in pixel
} __attribute__((packed)) l4re_video_color_component_t;

/**
 * \brief Pixel_info structure
 * \ingroup api_l4re_c_video
 */
typedef struct l4re_video_pixel_info_t
{
  l4re_video_color_component_t r, g, b, a; ///< Colors
  unsigned char bytes_per_pixel;           ///< Bytes per pixel
} l4re_video_pixel_info_t;

__BEGIN_DECLS

L4_INLINE L4_CV int
l4re_video_bits_per_pixel(l4re_video_pixel_info_t *p) L4_NOTHROW;

/* ************************************************************** */
/* Implementations */

L4_INLINE L4_CV int
l4re_video_bits_per_pixel(l4re_video_pixel_info_t *p) L4_NOTHROW
{
  return p->r.size + p->b.size + p->g.size + p->a.size;
}

__END_DECLS
