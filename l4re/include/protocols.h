/**
 * \file
 * L4Re Protocol Constants (C version)
 */

/*
 * (c) 2015 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

/**
 * \defgroup api_l4re_protocols L4Re Protocol identifiers
 * \ingroup api_l4re
 * Fix L4Re Protocol Constants.
 */

/**
 * Common L4Re Protocol Constants.
 * \ingroup api_l4re_protocols
 */
enum L4re_protocols
{
  L4RE_PROTO_DATASPACE = 0x4000, /**< ID for L4Re::Dataspace RPCs         */
  L4RE_PROTO_NAMESPACE,          /**< ID for L4Re::Namespace RPCs         */
  L4RE_PROTO_PARENT,             /**< ID for L4Re::Parent RPCs            */
  L4RE_PROTO_GOOS,               /**< ID for L4Re::Video::Goos RPCs       */
  L4RE_PROTO_RSVD_1,             /**< Reserved ID                         */
  L4RE_PROTO_RM,                 /**< ID for L4Re::Rm RPCs                */
  L4RE_PROTO_EVENT,              /**< ID for L4Re::Event RPCs             */
  L4RE_PROTO_INHIBITOR,          /**< ID for L4Re::Inhibitor RPCs         */
  L4RE_PROTO_DMA_SPACE,          /**< ID for L4Re::Dma_space RPCs         */
  L4RE_PROTO_MMIO_SPACE,         /**< ID for L4Re::Mmio_space             */
  L4RE_PROTO_ITAS,               /**< ID for L4Re::Itas                   */
  L4RE_PROTO_MEM_ALLOC,          /**< ID for L4Re::Mem_alloc              */
  L4RE_PROTO_REMOTE_ACCESS,      /**< ID for L4Re::Remote_access          */

  L4RE_PROTO_DEBUG = ~0x7fffL    /**< ID for debugging RPCs               */
};

