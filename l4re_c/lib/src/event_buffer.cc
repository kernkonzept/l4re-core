/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/env>
#include <l4/re/util/event_buffer>
#include <l4/re/c/event_buffer.h>

using L4Re::Util::Event_buffer_consumer;

static inline Event_buffer_consumer *cast(l4re_event_buffer_consumer_t *e) noexcept
{
  static_assert(sizeof(e->_obj_buf) >= sizeof(Event_buffer_consumer),
                "Argument event buffer too small");
  return reinterpret_cast<Event_buffer_consumer *>(&e->_obj_buf);
}

inline void *operator new(size_t, void *a) noexcept { return a; }

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
  auto *ev = reinterpret_cast<L4Re::Event_buffer::Event *>(e);

  ev->free();
}

L4_CV l4re_event_t *
l4re_event_buffer_next(l4re_event_buffer_consumer_t *evbuf) L4_NOTHROW
{
  return reinterpret_cast<l4re_event_t*>(cast(evbuf)->next());
}

L4_CV void
l4re_event_buffer_consumer_foreach_available_event
  (l4re_event_buffer_consumer_t *evbuf, void *data,
   l4re_event_buffer_cb_t *cb)
{
  typedef void Cb(L4Re::Event_buffer::Event*, void *);

  Cb *_cb = reinterpret_cast<Cb*>(cb);
  cast(evbuf)->foreach_available_event(_cb, data);
}

L4_CV void
l4re_event_buffer_consumer_process(l4re_event_buffer_consumer_t *evbuf,
                                   l4_cap_idx_t irq, l4_cap_idx_t thread, void *data,
                                   l4re_event_buffer_cb_t *cb)
{
  typedef void Cb(L4Re::Event_buffer::Event*, void *);

  L4::Cap<L4::Irq> i(irq);
  L4::Cap<L4::Thread> t(thread);

  Cb *_cb = reinterpret_cast<Cb*>(cb);
  cast(evbuf)->process(i, t, _cb, data);
}
