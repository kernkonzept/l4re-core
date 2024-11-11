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
 * License: see LICENSE.spdx (in this directory or the directories above)
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
  l4_addr_t       ldr_base;  /**< Load offset of executable */
} l4re_aux_t;

