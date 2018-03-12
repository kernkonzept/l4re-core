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

/**
 * \brief Flags of information on a view.
 * \ingroup api_l4re_c_video
 */
enum l4re_video_view_info_flags_t
{
  F_l4re_video_view_none               = 0x00, ///< everything for this view is static (the VESA-FB case)
  F_l4re_video_view_set_buffer         = 0x01, ///< buffer object for this view can be changed
  F_l4re_video_view_set_buffer_offset  = 0x02, ///< buffer offset can be set
  F_l4re_video_view_set_bytes_per_line = 0x04, ///< bytes per line can be set
  F_l4re_video_view_set_pixel          = 0x08, ///< pixel type can be set
  F_l4re_video_view_set_position       = 0x10, ///< position on screen can be set
  F_l4re_video_view_dyn_allocated      = 0x20, ///< View is dynamically allocated
  F_l4re_video_view_set_background     = 0x40, ///< Set view as background for session
  F_l4re_video_view_set_flags          = 0x80, ///< Set view property flags
  F_l4re_video_view_fully_dynamic      =   F_l4re_video_view_set_buffer
                                         | F_l4re_video_view_set_buffer_offset
                                         | F_l4re_video_view_set_bytes_per_line
                                         | F_l4re_video_view_set_pixel
                                         | F_l4re_video_view_set_position
                                         | F_l4re_video_view_dyn_allocated,

  F_l4re_video_view_above      = 0x01000, ///< Flag the view as stay on top
  F_l4re_video_view_flags_mask = 0xff000, ///< Mask containing all possible property flags
};

/**
 * \brief View information structure
 * \ingroup api_l4re_c_video
 */
typedef struct l4re_video_view_info_t
{
  unsigned                flags;                     ///< Flags
  unsigned                view_index;                ///< Number of view in the goos
  unsigned long           xpos, ypos, width, height; ///< Position in goos and size of view
  unsigned long           buffer_offset;             ///< Memory offset in goos buffer
  unsigned long           bytes_per_line;            ///< Size of line in view
  l4re_video_pixel_info_t pixel_info;                ///< Pixel info
  unsigned                buffer_index;              ///< Number of buffer of goos
} l4re_video_view_info_t;


/**
 * \brief C representation of a goos view.
 * \ingroup api_l4re_c_video
 *
 * A view is a visible rectangle that provides a view to the contents of a
 * buffer (frame buffer) memory object and is placed on a real screen.
 */
typedef struct l4re_video_view_t
{
  l4_cap_idx_t goos;
  unsigned idx;
} l4re_video_view_t;


EXTERN_C_BEGIN

/**
 * \ingroup api_l4re_c_video
 * \brief Flush the given rectangle of pixels of the given \a view.
 * \param view the target view of the operation.
 * \param x x-coordinate of the upper left corner
 * \param y y-coordinate of the upper left corner
 * \param w the width of the rectangle
 * \param h the height of the rectangle
 */
L4_CV int
l4re_video_view_refresh(l4re_video_view_t *view, int x, int y, int w,
                        int h) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Retrieve information about the given \a view.
 * \param view the target view for the operation.
 * \retval info a buffer receiving the information about the view.
 */
L4_CV int
l4re_video_view_get_info(l4re_video_view_t *view,
                         l4re_video_view_info_t *info) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Set properties of the view.
 * \param view the target view of the operation.
 * \param info the parameters to be set on the view.
 *
 * Which parameters can be manipulated on a given view can be figured out
 * with l4re_video_view_get_info() and this depends on the concrete instance
 * the view object.
 */
L4_CV int
l4re_video_view_set_info(l4re_video_view_t *view,
                         l4re_video_view_info_t *info) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Set the viewport parameters of a view.
 * \param view the target view of the operation.
 * \param x the x-coordinate of the upper left corner on the screen.
 * \param y the y-coordinate of the upper left corner on the screen.
 * \param w the width of the view.
 * \param h the height of the view.
 * \param bofs the offset (in bytes) of the upper left pixel in
 *             the memory buffer
 *
 * This function is a convenience wrapper for l4re_video_view_set_info(), just setting
 * the often changed parameters of a dynamic view. With this function a view
 * can be placed on the real screen and at the same time on its backing buffer.
 */
L4_CV int
l4re_video_view_set_viewport(l4re_video_view_t *view, int x, int y, int w,
                             int h, unsigned long bofs) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_video
 * \brief Change the stacking order in the stack of visible views.
 * \param view the target view for the operation.
 * \param pivot the neighbor view, relative to which \a view shall be stacked.
 *              a NULL value allows top (\a behind = 1) and bottom
 *              (\a behind = 0) placement of the view.
 * \param behind describes the placement of the view relative to the \a pivot view.
 */
L4_CV int
l4re_video_view_stack(l4re_video_view_t *view, l4re_video_view_t *pivot,
                      int behind) L4_NOTHROW;

EXTERN_C_END

