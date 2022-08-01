/**
 * \file
 * \brief DMA space C interface.
 */
/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \defgroup api_l4re_c_dma DMA Space Interface
 * \ingroup api_l4re_c
 * \brief DMA Space C interface.
 */

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>

L4_BEGIN_DECLS

/**
 * \copydoc L4Re::Dma_space::Direction
 */
enum l4re_dma_space_direction
{
  L4RE_DMA_SPACE_BIDIRECTIONAL, /**< device reads and writes to the memory */
  L4RE_DMA_SPACE_TO_DEVICE,     /**< device reads the memory */
  L4RE_DMA_SPACE_FROM_DEVICE,   /**< device writes to the memory */
  L4RE_DMA_SPACE_NONE           /**< device is coherently connected */
};

/**
 * \copydoc L4Re::Dma_space::Attribute
 */
enum l4re_dma_space_attributes
{
  L4RE_DMA_SPACE_SEARCH_ADDR = 1U << 0, ///< \copydoc L4Re::Dma_space::Search_addr
  L4RE_DMA_SPACE_PARTIAL_MAP = 1U << 1, ///< \copydoc L4Re::Dma_space::Partial_map
  L4RE_DMA_SPACE_RESERVE     = 1U << 2, ///< \copydoc L4Re::Dma_space::Reserve
  L4RE_DMA_SPACE_REPLACE     = 1U << 3, ///< \copydoc L4Re::Dma_space::Replace
};

/**
 * \copydoc L4Re::Dma_space_mgr::Space_attrib
 */
enum l4re_dma_space_mgr_space_attribs
{
  /// \copydoc L4Re::Dma_space_mgr::Identity_map
  L4RE_DMA_SPACE_MGR_IDENTITY_MAP = 1U << 0,
};

/**
 * \copydoc L4Re::Dma_space_mgr::Block_flag
 */
enum l4re_dma_space_mgr_block_flags
{
  /// \copydoc L4Re::Dma_space_mgr::Search_addr
  L4RE_DMA_SPACE_MGR_SEARCH_ADDR = 1U << 0,
};

/**
 * \brief DMA space capability type
 * \copydoc L4Re::Dma_space
 * \ingroup api_l4re_c_dma
 */
typedef l4_cap_idx_t l4re_dma_space_t;

/**
 * \brief DMA space manager capability type
 * \copydoc L4Re::Dma_space_mgr
 * \ingroup api_l4re_c_dma
 */
typedef l4_cap_idx_t l4re_dma_space_mgr_t;

/** Data type for DMA addresses. */
typedef l4_uint64_t l4re_dma_space_dma_addr_t;

/** Data type for DMA sizes. */
typedef l4_uint64_t l4re_dma_space_dma_size_t;

/**
 * \copybrief L4Re::Dma_space::map
 * \param dma  DMA space capability
 * \copydetails L4Re::Dma_space::map
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_map(l4re_dma_space_t dma, l4re_ds_t src,
                   l4re_ds_offset_t offset,
                   l4_size_t * size, unsigned long attrs,
                   enum l4re_dma_space_direction dir,
                   l4re_dma_space_dma_addr_t *dma_addr) L4_NOTHROW;


/**
 * \copybrief L4Re::Dma_space::unmap
 * \param dma  DMA space capability
 * \copydetails L4Re::Dma_space::unmap
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_unmap(l4re_dma_space_t dma, l4re_dma_space_dma_addr_t dma_addr,
                     l4_size_t size, unsigned long attrs,
                     enum l4re_dma_space_direction dir) L4_NOTHROW;


/**
 * \copybrief L4Re::Dma_space_mgr::associate
 * \param dma_mgr  DMA space manager capability
 * \copydetails L4Re::Dma_space_mgr::associate
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_mgr_associate(l4re_dma_space_mgr_t dma_mgr,
                             l4re_dma_space_t dma_space,
                             l4_cap_idx_t dma_task,
                             enum l4re_dma_space_mgr_space_attribs attr) L4_NOTHROW;

/**
 * \copybrief L4Re::Dma_space_mgr::associate_phys
 * \param dma_mgr  DMA space manager capability
 * \copydetails L4Re::Dma_space_mgr::associate_phys
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_mgr_associate_phys(l4re_dma_space_mgr_t dma_mgr,
                                  l4re_dma_space_t dma_space,
                                  enum l4re_dma_space_mgr_space_attribs attr) L4_NOTHROW;

/**
 * \param dma_mgr   DMA space manager capability
 * \copydoc L4Re::Dma_space_mgr::disassociate
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_mgr_disassociate(l4re_dma_space_mgr_t dma_mgr,
                                l4re_dma_space_t dma_space) L4_NOTHROW;

/**
 * \param dma_mgr   DMA space manager capability
 * \copydoc L4Re::Dma_space_mgr::block_area
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_mgr_block_area(l4re_dma_space_mgr_t dma_mgr,
                              l4re_dma_space_t dma_space,
                              l4re_dma_space_dma_addr_t *addr,
                              l4re_dma_space_dma_size_t size,
                              l4re_dma_space_dma_addr_t max_addr,
                              enum l4re_dma_space_mgr_block_flags flags,
                              unsigned char align) L4_NOTHROW;

/**
 * \param dma_mgr   DMA space manager capability
 * \copydoc L4Re::Dma_space_mgr::set_limits
 * \ingroup api_l4re_c_dma
 */
L4_CV l4_ret_t
l4re_dma_space_mgr_set_limits(l4re_dma_space_mgr_t dma_mgr,
                              l4re_dma_space_t dma_space,
                              l4re_dma_space_dma_addr_t min_addr,
                              l4re_dma_space_dma_addr_t max_addr) L4_NOTHROW;

L4_END_DECLS
