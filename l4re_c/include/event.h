/**
 * \file
 * \brief Event C interface.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \defgroup api_l4re_c_event Event interface
 * \ingroup api_l4re_c
 * \brief Event C interface.
 */

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>
#include <l4/re/event.h>

L4_BEGIN_DECLS

/**
 * \brief Event structure used in buffer.
 */
typedef struct
{
  long long time;         /**< Time stamp of the event */
  unsigned short type;    /**< Type of the event */
  unsigned short code;    /**< Code of the event */
  int value;              /**< Value of the event */
  l4_umword_t stream_id;  /**< Stream ID */
} l4re_event_t;

/**
 * \brief Get an event signal buffer.
 * \ingroup api_l4re_c_event
 *
 * \param server   Server to talk to.
 * \param ds       Buffer to event data.
 *
 * \return 0 for success, <0 on error
 *
 * \see L4Re::Event::get_buffer
 */
L4_CV l4_ret_t
l4re_event_get_buffer(const l4_cap_idx_t server,
                      const l4re_ds_t ds) L4_NOTHROW;

/**
 * \brief Get number of streams
 * \ingroup api_l4re_c_event
 *
 * \param server   Server to talk to.
 *
 * \return 0 for success, <0 on error
 *
 * \see L4Re::Event::get_num_streams
 */
L4_CV l4_ret_t
l4re_event_get_num_streams(const l4_cap_idx_t server) L4_NOTHROW;

/**
 * \brief Get information on a stream
 * \ingroup api_l4re_c_event
 *
 * \param server     Server to talk to.
 * \param idx        Index value.
 * \param[out] info  Information buffer.
 *
 * \return 0 for success, <0 on error
 *
 * \see L4Re::Event::get_stream_info
 */
L4_CV l4_ret_t
l4re_event_get_stream_info(const l4_cap_idx_t server,
                           int idx, l4re_event_stream_info_t *info) L4_NOTHROW;

/**
 * \brief Get info for a stream given a stream id
 * \ingroup api_l4re_c_event
 *
 * \param      server     Server to talk to.
 * \param      stream_id  Stream ID.
 * \param[out] info       Information buffer.
 *
 * \return 0 for success, <0 on error
 *
 * \see L4Re::Event::get_stream_info_for_id
 */
L4_CV l4_ret_t
l4re_event_get_stream_info_for_id(const l4_cap_idx_t server,
                                  l4_umword_t stream_id,
                                  l4re_event_stream_info_t *info) L4_NOTHROW;

/**
 * \brief Get Axis information for a stream.
 * \ingroup api_l4re_c_event
 *
 * \param      server  Server to talk to.
 * \param      id      Id of the stream to get information from.
 * \param      naxes   Number of axes in `axis` array.
 * \param[in]  axis    Array of axis IDs whose information should be retrieved.
 * \param[out] info    Information buffer to store the retrieved axis infos.
 *
 * \retval 0   Success
 * \retval <0  Error
 *
 * \see L4Re::Event::get_axis_info
 */
L4_CV l4_ret_t
l4re_event_get_axis_info(const l4_cap_idx_t server, l4_umword_t id,
                         unsigned naxes, unsigned const *axis,
                         l4re_event_absinfo_t *info) L4_NOTHROW;

L4_END_DECLS
