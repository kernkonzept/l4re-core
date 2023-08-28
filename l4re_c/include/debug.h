/**
 * \file
 * \brief Debug C interface.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \defgroup api_l4re_c_debug Debug interface
 * \ingroup api_l4re_c
 */

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>

L4_BEGIN_DECLS

/**
 * \brief Call debug function of L4Re service
 * \ingroup api_l4re_c_debug
 * \param  srv       Object to call.
 * \param  function  Function to call.
 * \see L4Re::Debug_obj::debug
 */
L4_CV l4_ret_t
l4re_debug_obj_debug(l4_cap_idx_t srv, unsigned long function) L4_NOTHROW;

L4_END_DECLS
