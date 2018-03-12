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
#include <l4/re/env>
#include <l4/re/util/event_buffer>
#include <l4/re/c/event_buffer.h>

using L4Re::Util::Event_buffer_consumer;

static inline Event_buffer_consumer *cast(l4re_event_buffer_consumer_t *e) throw()
{
  (void)sizeof(char[sizeof(e->_obj_buf) - sizeof(Event_buffer_consumer)]);
  return (Event_buffer_consumer *)e->_obj_buf;
}

inline void *operator new(size_t, void *a) throw() { return a; }

L4_CV long
l4re_event_buffer_attach(l4re_event_buffer_consumer_t *evbuf,
                         l4re_ds_t ds, l4_cap_idx_t rm) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> d(ds);
  L4::Cap<L4Re::Rm>        r(rm);
  new (evbuf->_obj_buf) Event_buffer_consumer();
  return cast(evbuf)->attach(d, r);
}

L4_CV long
l4re_event_buffer_detach(l4re_event_buffer_consumer_t *evbuf,
                         l4_cap_idx_t rm) L4_NOTHROW
{
  L4::Cap<L4Re::Rm>        r(rm);
  return cast(evbuf)->detach(r);
}

L4_CV void
l4re_event_free(l4re_event_t *e) L4_NOTHROW
{
  L4Re::Event_buffer::Event *ev = (L4Re::Event_buffer::Event *)e;

  ev->free();
}

L4_CV l4re_event_t *
l4re_event_buffer_next(l4re_event_buffer_consumer_t *evbuf) L4_NOTHROW
{
  return (l4re_event_t*)cast(evbuf)->next();
}

L4_CV void
l4re_event_buffer_consumer_foreach_available_event
  (l4re_event_buffer_consumer_t *evbuf, void *data,
   l4re_event_buffer_cb_t *cb)
{
  typedef void Cb(L4Re::Event_buffer::Event*, void *);

  Cb *_cb = (Cb*)cb;
  cast(evbuf)->foreach_available_event(_cb, data);
}

L4_CV void
l4re_event_buffer_consumer_process(l4re_event_buffer_consumer_t *evbuf,
                                   l4_cap_idx_t irq, l4_cap_idx_t thread, void *data,
                                   l4re_event_buffer_cb_t *cb)
{
  L4::Cap<L4::Irq> i(irq);
  L4::Cap<L4::Thread> t(thread);
  void (*_cb)(L4Re::Event_buffer::Event*, void *)
    = (void (*)(L4Re::Event_buffer::Event*, void *))cb;
  cast(evbuf)->process(i, t, _cb, data);
}
