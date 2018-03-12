#pragma once
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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

#include <l4/sys/linkage.h>
#include <l4/re/c/event.h>

EXTERN_C_BEGIN

typedef struct l4re_event_buffer_consumer_t
{
  unsigned long _obj_buf[8];
} l4re_event_buffer_consumer_t;

L4_CV void
l4re_event_free(l4re_event_t *e) L4_NOTHROW;

L4_CV long
l4re_event_buffer_attach(l4re_event_buffer_consumer_t *evbuf,
                         l4re_ds_t ds, l4_cap_idx_t rm) L4_NOTHROW;

L4_CV long
l4re_event_buffer_detach(l4re_event_buffer_consumer_t *evbuf,
                         l4_cap_idx_t rm) L4_NOTHROW;

L4_CV l4re_event_t *
l4re_event_buffer_next(l4re_event_buffer_consumer_t *evbuf) L4_NOTHROW;

typedef L4_CV void l4re_event_buffer_cb_t(l4re_event_t *ev, void *data);

L4_CV void
l4re_event_buffer_consumer_foreach_available_event(l4re_event_buffer_consumer_t *evbuf,
    void *data, l4re_event_buffer_cb_t *cb);


L4_CV void
l4re_event_buffer_consumer_process(l4re_event_buffer_consumer_t *evbuf,
                                   l4_cap_idx_t irq, l4_cap_idx_t thread, void *data,
                                   l4re_event_buffer_cb_t *cb);

EXTERN_C_END
