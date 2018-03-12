/**
 * \file
 * \brief   Events
 */
/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

typedef struct L4_EXPORT_TYPE l4re_event_stream_id_t
{
  l4_uint16_t bustype;
  l4_uint16_t vendor;
  l4_uint16_t product;
  l4_uint16_t version;
} l4re_event_stream_id_t;

typedef struct L4_EXPORT_TYPE l4re_event_absinfo_t
{
  l4_int32_t value;
  l4_int32_t min;
  l4_int32_t max;
  l4_int32_t fuzz;
  l4_int32_t flat;
  l4_int32_t resolution;
} l4re_event_absinfo_t;

enum l4re_event_stream_max_values_t
{
  L4RE_EVENT_EV_MAX  = 0x1f,
  L4RE_EVENT_KEY_MAX = 0x1ff,
  L4RE_EVENT_REL_MAX = 0xf,
  L4RE_EVENT_ABS_MAX = 0x3f,
  L4RE_EVENT_PROP_MAX = 0x1f,
  L4RE_EVENT_SW_MAX   = 0xf, // should be >= L4RE_SW_MAX
};

enum l4re_event_stream_props_t
{
  L4RE_EVENT_STREAM_CALIBRATE = 0x001,
};


#define __UNUM_B(x) ((x+1) + sizeof(unsigned long)*8 - 1) / (sizeof(unsigned long)*8)

typedef struct L4_EXPORT_TYPE l4re_event_stream_info_t
{
  l4_umword_t stream_id;
  char name[32];
  char phys[32];
  l4re_event_stream_id_t id;

  unsigned long propbits[__UNUM_B(L4RE_EVENT_PROP_MAX)];

  unsigned long evbits[__UNUM_B(L4RE_EVENT_EV_MAX)];
  unsigned long keybits[__UNUM_B(L4RE_EVENT_KEY_MAX)];
  unsigned long relbits[__UNUM_B(L4RE_EVENT_REL_MAX)];
  unsigned long absbits[__UNUM_B(L4RE_EVENT_ABS_MAX)];
  unsigned long swbits[__UNUM_B(L4RE_EVENT_SW_MAX)];

} l4re_event_stream_info_t;

typedef struct L4_EXPORT_TYPE l4re_event_stream_state_t
{
  unsigned long keybits[__UNUM_B(L4RE_EVENT_KEY_MAX)];
  unsigned long swbits[__UNUM_B(L4RE_EVENT_SW_MAX)];
} l4re_event_stream_state_t;

#undef __UNUM_B

