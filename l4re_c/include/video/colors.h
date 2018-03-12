/**
 * \file
 * \note The C interface of L4Re::Video does _NOT_ reflect the full C++
 *       interface on purpose. Use the C++ interface where possible.
 */
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

EXTERN_C_BEGIN

L4_INLINE L4_CV int
l4re_video_bits_per_pixel(l4re_video_pixel_info_t *p) L4_NOTHROW;

/* ************************************************************** */
/* Implementations */

L4_INLINE L4_CV int
l4re_video_bits_per_pixel(l4re_video_pixel_info_t *p) L4_NOTHROW
{
  return p->r.size + p->b.size + p->g.size + p->a.size;
}

EXTERN_C_END
