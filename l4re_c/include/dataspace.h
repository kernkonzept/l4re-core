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
typedef l4_uint64_t l4re_ds_size_t;
typedef l4_uint64_t l4re_ds_offset_t;
typedef l4_uint64_t l4re_ds_map_addr_t;
typedef unsigned long l4re_ds_flags_t;

/**
 * \brief Information about the data space.
 * \ingroup api_l4re_c_ds
 */
typedef struct {
  l4re_ds_size_t size;    ///< size
  l4re_ds_flags_t flags;  ///< flags
} l4re_ds_stats_t;

/**
 * Flags to specify the memory mapping type of a request
 * \ingroup api_l4re_c_ds
 */
enum l4re_ds_map_flags {
  L4RE_DS_F_R   = L4_FPAGE_RO,
  L4RE_DS_F_W   = L4_FPAGE_W,
  L4RE_DS_F_X   = L4_FPAGE_X,
  L4RE_DS_F_RW  = L4_FPAGE_RW,
  L4RE_DS_F_RX  = L4_FPAGE_RX,
  L4RE_DS_F_RWX = L4_FPAGE_RWX,

  L4RE_DS_F_RIGHTS_MASK = 0x0f,

  L4RE_DS_F_NORMAL        = 0x00, ///< request normal memory mapping
  L4RE_DS_F_CACHEABLE     = L4RE_DS_F_NORMAL, ///< request normal memory mapping
  L4RE_DS_F_BUFFERABLE    = 0x10, ///< request bufferable (write buffered) mappings
  L4RE_DS_F_UNCACHEABLE   = 0x20, ///< request uncacheable memory mappings
  L4RE_DS_F_CACHING_MASK  = 0x30, ///< mask for caching flags
  L4RE_DS_F_CACHING_SHIFT = 4,    ///< shift value for caching flags
};

/**
 * \internal
 * \ingroup api_l4re_c_ds
 * \see L4Re::Dataspace::map
 */
L4_CV int
l4re_ds_map(l4re_ds_t ds,
            l4re_ds_offset_t offset,
            l4re_ds_flags_t flags,
            l4re_ds_map_addr_t local_addr,
            l4re_ds_map_addr_t min_addr,
            l4re_ds_map_addr_t max_addr) L4_NOTHROW;

/**
 * \internal
 * \ingroup api_l4re_c_ds
 * \see L4Re::Dataspace::map_page
 */
L4_CV int
l4re_ds_map_region(l4re_ds_t ds,
                   l4re_ds_offset_t offset,
                   l4re_ds_flags_t flags,
                   l4re_ds_map_addr_t min_addr,
                   l4re_ds_map_addr_t max_addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::clear
 * \param ds  Dataspace capability.
 * \copydetails L4Re::Dataspace::clear
 */
L4_CV long
l4re_ds_clear(l4re_ds_t ds, l4re_ds_offset_t offset,
              l4re_ds_size_t size) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::allocate
 * \param ds  Dataspace capability.
 * \copydetails L4Re::Dataspace::allocate
 */
L4_CV long
l4re_ds_allocate(l4re_ds_t ds,
                 l4re_ds_offset_t offset,
                 l4re_ds_size_t size) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::copy_in
 * \param ds  Destination dataspace.
 * \copydetails L4Re::Dataspace::copy_in
 */
L4_CV int
l4re_ds_copy_in(l4re_ds_t ds, l4re_ds_offset_t dst_offs,
                l4re_ds_t src, l4re_ds_offset_t src_offs,
                l4re_ds_size_t size) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::size
 * \param ds  Dataspace capability.
 * \copydetails L4Re::Dataspace::size
 */
L4_CV l4re_ds_size_t
l4re_ds_size(l4re_ds_t ds) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::flags
 * \param ds  Dataspace capability.
 * \copydetails L4Re::Dataspace::flags
 */
L4_CV l4re_ds_flags_t
l4re_ds_flags(l4re_ds_t ds) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::info
 * \param ds  Dataspace capability.
 * \copydetails L4Re::Dataspace::info
 */
L4_CV int
l4re_ds_info(l4re_ds_t ds, l4re_ds_stats_t *stats) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ds
 * \copybrief L4Re::Dataspace::map_info
 * \param ds  Dataspace capability.
 * \copydetails L4Re::Dataspace::map_info
 */
L4_CV int
l4re_ds_map_info(l4re_ds_t ds,
                 l4_addr_t *start_addr, l4_addr_t *end_addr) L4_NOTHROW;

EXTERN_C_END
