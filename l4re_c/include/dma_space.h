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

#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>

__BEGIN_DECLS

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
 * \copydoc L4Re::Dma_space::Space_attrib
 */
enum l4re_dma_space_space_attribs
{
  L4RE_DMA_SPACE_COHERENT   = 1 << 0, ///< \copydoc L4Re::Dma_space::Coherent
  L4RE_DMA_SPACE_PHYS_SPACE = 1 << 1, ///< \copydoc L4Re::Dma_space::Phys_space
};

/**
 * \brief DMA space capability type
 * \copydoc L4Re::Dma_space
 * \ingroup api_l4re_c_dma
 */
typedef l4_cap_idx_t l4re_dma_space_t;

/** Data type for DMA addresses. */
typedef l4_uint64_t l4re_dma_space_dma_addr_t;

/**
 * \copybrief L4Re::Dma_space::map
 * \param dma  DMA space capability
 * \copydetails L4Re::Dma_space::map
 * \ingroup api_l4re_c_dma
 */
L4_CV long
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
L4_CV long
l4re_dma_space_unmap(l4re_dma_space_t dma, l4re_dma_space_dma_addr_t dma_addr,
                     l4_size_t size, unsigned long attrs,
                     enum l4re_dma_space_direction dir) L4_NOTHROW;

/**
 * \copybrief L4Re::Dma_space::associate
 * \param dma  DMA space capability
 * \copydetails L4Re::Dma_space::associate
 * \ingroup api_l4re_c_dma
 */
L4_CV long
l4re_dma_space_associate(l4re_dma_space_t dma, l4_cap_idx_t dma_task,
                         unsigned long attr) L4_NOTHROW;

/**
 * \copybrief L4Re::Dma_space::disassociate
 * \param dma  DMA space capability
 * \copydetails L4Re::Dma_space::disassociate
 * \ingroup api_l4re_c_dma
 */
L4_CV long
l4re_dma_space_disassociate(l4re_dma_space_t dma);


__END_DECLS
