/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 */
#pragma once

#include <l4/sys/l4int.h>

/** Flags for \c l4util_l4mod_mod.flags */
enum l4util_l4mod_mod_info_flag // aka bootstraps: Mod_info_flags
{
  L4util_l4mod_mod_flag_unspec    = 0,
  L4util_l4mod_mod_flag_kernel    = 1,
  L4util_l4mod_mod_flag_sigma0    = 2,
  L4util_l4mod_mod_flag_roottask  = 3,
  L4util_l4mod_mod_flag_mask      = 7 << 0,
};

/** A single module. */
typedef struct
{
  l4_uint64_t flags;       /**< Module flags (\c l4util_l4mod_mod_info_flag) */
  l4_uint64_t mod_start;   /**< Starting address of module in memory. */
  l4_uint64_t mod_end;     /**< End address of module in memory. */
  l4_uint64_t cmdline;     /**< Module command line */
} l4util_l4mod_mod;

/** Base module structure. */
typedef struct
{
  l4_uint64_t flags;          /**< Flags */
  l4_uint64_t cmdline;        /**< Pointer to kernel command line */
  l4_uint64_t mods_addr;      /**< Module list */
  l4_uint32_t mods_count;     /**< Number of modules */
  l4_uint32_t _pad;

  /**
   * VESA video info, valid if one of vbe_ctrl_info
   * or vbe_mode_info is not zero.
   */
  l4_uint64_t vbe_ctrl_info;  /**< VESA video controller info */
  l4_uint64_t vbe_mode_info;  /**< VESA video mode info */
} l4util_l4mod_info;
