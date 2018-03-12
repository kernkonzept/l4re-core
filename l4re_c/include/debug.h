/**
 * \file
 * \brief Debug C interface.
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

/**
 * \defgroup api_l4re_c_debug Debug interface
 * \ingroup api_l4re_c
 */

#include <l4/sys/types.h>

EXTERN_C_BEGIN

/**
 * \brief Call debug function of L4Re service
 * \ingroup api_l4re_c_debug
 * \param  srv       Object to call.
 * \param  function  Function to call.
 * \see L4Re::Debug_obj::debug
 */
L4_CV void
l4re_debug_obj_debug(l4_cap_idx_t srv, unsigned long function) L4_NOTHROW;

EXTERN_C_END
