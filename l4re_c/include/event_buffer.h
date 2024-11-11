#pragma once
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/sys/linkage.h>
#include <l4/re/c/event.h>

__BEGIN_DECLS

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

__END_DECLS
