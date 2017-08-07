/**
 * \file
 * \brief DMA space C interface.
 */
/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
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
 * \defgroup api_l4re_c_dma DMA Space Interface
 * \ingroup api_l4re_c
 * \brief DMA Space C interface.
 */

#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>

EXTERN_C_BEGIN

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
  L4RE_DMA_SPACE_COHERENT   = 1 << 0, /**< \copydoc L4Re::Dma_space::Space_attrib::Coherent */
  L4RE_DMA_SPACE_PHYS_SPACE = 1 << 1, /**< \copydoc L4Re::Dma_space::Space_attrib::Phys_space */
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
 * \param dma  DMA space capability
 * \copydoc L4Re::Dma_space::map
 * \ingroup api_l4re_c_dma
 */
L4_CV long
l4re_dma_space_map(l4re_dma_space_t dma, l4re_ds_t src, l4_addr_t offset,
                   l4_size_t * size, unsigned long attrs,
                   enum l4re_dma_space_direction dir,
                   l4re_dma_space_dma_addr_t *dma_addr) L4_NOTHROW;


/**
 * \param dma  DMA space capability
 * \copydoc L4Re::Dma_space::unmap
 * \ingroup api_l4re_c_dma
 */
L4_CV long
l4re_dma_space_unmap(l4re_dma_space_t dma, l4re_dma_space_dma_addr_t dma_addr,
                     l4_size_t size, unsigned long attrs,
                     enum l4re_dma_space_direction dir) L4_NOTHROW;

/**
 * \param dma  DMA space capability
 * \copydoc L4Re::Dma_space::associate
 * \ingroup api_l4re_c_dma
 */
L4_CV long
l4re_dma_space_associate(l4re_dma_space_t dma, l4_cap_idx_t dma_task,
                         unsigned long attr) L4_NOTHROW;

/**
 * \param dma  DMA space capability
 * \copydoc L4Re::Dma_space::disassociate
 * \ingroup api_l4re_c_dma
 */
L4_CV long
l4re_dma_space_disassociate(l4re_dma_space_t dma);


EXTERN_C_END

