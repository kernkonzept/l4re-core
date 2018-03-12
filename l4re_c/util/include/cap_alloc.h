/**
 * \file
 * \brief   Capability allocator C interface
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
 * \defgroup api_l4re_c_util_cap Capability allocator
 * \ingroup api_l4re_c
 * \brief Capability allocator C interface.
 */

#include <l4/sys/types.h>
#include <l4/sys/linkage.h>

EXTERN_C_BEGIN

/**
 * \brief Get free capability index at capability allocator
 * \ingroup api_l4re_c_util_cap
 */
L4_CV l4_cap_idx_t
l4re_util_cap_alloc(void) L4_NOTHROW;

/**
 * \brief Return capability index to capability allocator
 * \ingroup api_l4re_c_util_cap
 */
L4_CV void
l4re_util_cap_free(l4_cap_idx_t cap) L4_NOTHROW;

/**
 * \brief Return capability index to capability allocator, and unmaps the
 *        object.
 * \ingroup api_l4re_c_util_cap
 */
L4_CV void
l4re_util_cap_free_um(l4_cap_idx_t cap) L4_NOTHROW;

/**
 * \brief Return last capability index the allocator can return
 * \return last/biggest capability index the allocator can return
 * \ingroup api_l4re_c_util_cap
 */
L4_CV long
l4re_util_cap_last(void) L4_NOTHROW;

EXTERN_C_END
