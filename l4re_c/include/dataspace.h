/**
 * \file
 * \brief Data space C interface.
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
 * \defgroup api_l4re_c_ds Dataspace interface
 * \ingroup api_l4re_c
 * \brief Dataspace C interface.
 */

#include <l4/sys/types.h>

EXTERN_C_BEGIN

/**
 * \brief Dataspace type
 * \ingroup api_l4re_c_ds
 */
typedef l4_cap_idx_t l4re_ds_t;

/**
 * \brief Information about the data space.
 * \ingroup api_l4re_c_ds
 */
typedef struct {
  unsigned long size;   ///< size
  unsigned long flags;  ///< flags
} l4re_ds_stats_t;

/**
 * Flags to specify the memory mapping type of a request
 * \ingroup api_l4re_c_ds
 */
enum l4re_ds_map_flags {
  L4RE_DS_MAP_F_R = 1,  ///< Request readable mapping.
  L4RE_DS_MAP_F_W = 2,  ///< Request writable mapping.
  L4RE_DS_MAP_F_X = 4,  ///< Request executable mapping.

  /** Request read-only mapping. */
  L4RE_DS_MAP_F_RO  = L4RE_DS_MAP_F_R,
  /** Request read-writable mapping. */
  L4RE_DS_MAP_F_RW  = L4RE_DS_MAP_F_R | L4RE_DS_MAP_F_W,
  /** Request read-executable mapping. */
  L4RE_DS_MAP_F_RX  = L4RE_DS_MAP_F_R | L4RE_DS_MAP_F_X,
  /** Request read-write-executable mapping. */
  L4RE_DS_MAP_F_RWX = L4RE_DS_MAP_F_RW | L4RE_DS_MAP_F_X,

  L4RE_DS_MAP_NORMAL        = 0x00, ///< request normal memory mapping
  L4RE_DS_MAP_CACHEABLE     = L4RE_DS_MAP_NORMAL, ///< request normal memory mapping
  L4RE_DS_MAP_BUFFERABLE    = 0x10, ///< request bufferable (write buffered) mappings
  L4RE_DS_MAP_UNCACHEABLE   = 0x20, ///< request uncacheable memory mappings
  L4RE_DS_MAP_CACHING_MASK  = 0x30, ///< mask for caching flags
  L4RE_DS_MAP_CACHING_SHIFT = 4,    ///< shift value for caching flags
};

/**
 * \internal
 * \ingroup api_l4re_c_ds
 * \see L4Re::Dataspace::map
 */
L4_CV int
l4re_ds_map(const l4re_ds_t ds, l4_addr_t offset, unsigned long flags,
            l4_addr_t local_addr, l4_addr_t min_addr, l4_addr_t max_addr) L4_NOTHROW;

/**
 * \internal
 * \ingroup api_l4re_c_ds
 * \see L4Re::Dataspace::map_page
 */
L4_CV int
l4re_ds_map_region(const l4re_ds_t ds, l4_addr_t offset, unsigned long flags,
                   l4_addr_t min_addr, l4_addr_t max_addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 *
 * \return 0 on success, <0 on errors
 * \see L4Re::Dataspace::clear
 */
L4_CV long
l4re_ds_clear(const l4re_ds_t ds, l4_addr_t offset, unsigned long size) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \return 0 on success, <0 on errors
 * \see L4Re::Dataspace::allocate
 */
L4_CV long
l4re_ds_allocate(const l4re_ds_t ds,
                 l4_addr_t offset, l4_size_t size) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \return 0 on success, <0 on errors
 * \see L4Re::Dataspace::copy_in
 */
L4_CV int
l4re_ds_copy_in(const l4re_ds_t ds, l4_addr_t dst_offs, const l4re_ds_t src,
                l4_addr_t src_offs, unsigned long size) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \return Size of the dataspace in bytes.
 * \see L4Re::Dataspace::size
 */
L4_CV unsigned long
l4re_ds_size(const l4re_ds_t ds) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \see L4Re::Dataspace::flags
 */
L4_CV long
l4re_ds_flags(const l4re_ds_t ds) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \see L4Re::Dataspace::info
 */
L4_CV int
l4re_ds_info(const l4re_ds_t ds, l4re_ds_stats_t *stats) L4_NOTHROW;

EXTERN_C_END
