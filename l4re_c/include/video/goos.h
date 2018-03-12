/**
 * \file
 * \note The C interface of L4Re::Video does _NOT_ reflect the full C++
 *       interface on purpose. Use the C++ where possible.
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

#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>
#include <l4/re/c/video/colors.h>
#include <l4/re/c/video/view.h>

/**
 * \defgroup api_l4re_c_video Video API
 * \ingroup api_l4re_c
 */

/**
 * \brief Flags of information on the goos.
 * \ingroup api_l4re_c_video
 */
enum l4re_video_goos_info_flags_t
{
  F_l4re_video_goos_auto_refresh    = 0x01, ///< The graphics display is automatically refreshed
  F_l4re_video_goos_pointer         = 0x02, ///< We have a mouse pointer
  F_l4re_video_goos_dynamic_views   = 0x04, ///< Supports dynamically allocated views
  F_l4re_video_goos_dynamic_buffers = 0x08, ///< Supports dynamically allocated buffers
};

/**
 * \brief Goos information structure
 * \ingroup api_l4re_c_video
 */
typedef struct
{
  unsigned long width;                ///< Width of the goos
  unsigned long height;               ///< Height of the goos
  unsigned flags;                     ///< Flags of the framebuffer, see #l4re_video_goos_info_flags_t
  unsigned num_static_views;          ///< Number of static views
  unsigned num_static_buffers;        ///< Number of static buffers
  l4re_video_pixel_info_t pixel_info; ///< Pixel layout of the goos
} l4re_video_goos_info_t;

/**
 * \brief Goos object type
 * \ingroup api_l4re_c_fb
 */
typedef l4_cap_idx_t l4re_video_goos_t;

EXTERN_C_BEGIN

/**
 * \brief Get information on a goos.
 * \ingroup api_l4re_c_video
 *
 * \param  goos  Goos object
 * \retval ginfo Pointer to goos information structure.
 *
 * \return 0 for success, <0 on error
 *         - -#L4_ENODEV
 *         - IPC errors
 */
L4_CV int
l4re_video_goos_info(l4re_video_goos_t goos,
                     l4re_video_goos_info_t *ginfo) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Flush a rectangle of pixels of the goos screen.
 * \param goos the target object of the operation.
 * \param x the x-coordinate of the upper left corner of the rectangle
 * \param y the y-coordinate of the upper left corner of the rectangle
 * \param w the width of the rectangle to be flushed
 * \param h the height of the rectangle
 */
L4_CV int
l4re_video_goos_refresh(l4re_video_goos_t goos, int x, int y, int w,
                        int h) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Create a new buffer (memory buffer) for pixel data.
 * \param goos the target object for the operation.
 * \param size the size in bytes for the pixel buffer.
 * \param buffer a capability index to receive the data-space capability
 *               for the buffer.
 * \return >=0: The index of the created buffer (used to assign views
 *               and for deletion).
 *         < 0: on error
 */
L4_CV int
l4re_video_goos_create_buffer(l4re_video_goos_t goos, unsigned long size,
                              l4_cap_idx_t buffer) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Delete a pixel buffer.
 * \param goos the target goos object.
 * \param idx the buffer index of the buffer to delete (the return
 *            value of l4re_video_goos_create_buffer())
 */
L4_CV int
l4re_video_goos_delete_buffer(l4re_video_goos_t goos, unsigned idx) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Get the data-space capability of the static pixel buffer.
 * \param goos    The target goos object.
 * \param idx     Index of the static buffer.
 * \param buffer  A capability index to receive the data-space capability.
 *
 * This function allows access to static, preexisting pixel buffers. Such static buffers
 * exist for static configurations, such as the VESA framebuffer.
 */
L4_CV int
l4re_video_goos_get_static_buffer(l4re_video_goos_t goos, unsigned idx,
                                  l4_cap_idx_t buffer) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Create a new view (\see l4re_video_view_t)
 * \param goos the goos session to use.
 * \retval view the structure will be initialized for the new view.
 */
L4_CV int
l4re_video_goos_create_view(l4re_video_goos_t goos,
                            l4re_video_view_t *view) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Delete a view.
 * \param goos the goos session to use.
 * \param view the view to delete, the given data-structure is invalid
 *             afterwards.
 */
L4_CV int
l4re_video_goos_delete_view(l4re_video_goos_t goos,
                            l4re_video_view_t *view) L4_NOTHROW;


/**
 * \ingroup api_l4re_c_video
 * \brief Get a view for the given index.
 * \param goos the target goos session.
 * \param idx the index of the view to retrieve.
 * \retval view the structure will be initialized to the
 *              view with the given index.
 *
 * This function allows to access static views as provided by the
 * VESA framebuffer (the monitor). However, it also allows to access
 * dynamic views created with l4re_video_goos_create_view().
 */
L4_CV int
l4re_video_goos_get_view(l4re_video_goos_t goos, unsigned idx,
                         l4re_video_view_t *view) L4_NOTHROW;

EXTERN_C_END
