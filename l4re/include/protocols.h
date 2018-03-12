/**
 * L4Re Protocol Constants (C version)
 */

/*
 * (c) 2015 Alexander Warg <alexander.warg@kernkonzept.com>
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
 * Fix L4Re Protocol Constants.
 * \defgroup api_l4re_protocols L4Re Protocol identifiers
 * \ingroup api_l4re
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

  L4RE_PROTO_DEBUG = ~0x7fffL    /**< ID for debugging RPCs               */
};

