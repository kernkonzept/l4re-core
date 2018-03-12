#pragma once
/**
 * \file
 * \brief   Auxiliary definitions
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
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

#include <l4/sys/types.h>

/**
 * \defgroup api_l4aux Auxiliary data
 * \ingroup api_l4re
 * \internal
 */

/**
 * \brief Flags for program loading
 * \ingroup api_l4aux
 * \internal
 */
enum l4re_aux_ldr_flags_t
{
  L4RE_AUX_LDR_FLAG_EAGER_MAP    = 0x1,
  L4RE_AUX_LDR_FLAG_ALL_SEGS_COW = 0x2,
  L4RE_AUX_LDR_FLAG_PINNED_SEGS  = 0x4,
};

/**
 * \brief Auxiliary descriptor
 * \ingroup api_l4aux
 * \internal
 */
typedef struct l4re_aux_t
{
  char const *    binary;    /**< Binary name */
  l4_cap_idx_t    kip_ds;    /**< Data space of the KIP */
  l4_umword_t     dbg_lvl;   /**< Debug levels for l4re */
  l4_umword_t     ldr_flags; /**< Flags for l4re, see \a l4re_aux_ldr_flags_t */
} l4re_aux_t;

