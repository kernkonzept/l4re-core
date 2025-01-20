/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*
 * Message handler/dispatcher
 */

#include "debug.h"
#include "dispatcher.h"
#include "globals.h"
#include "region.h"

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/unique_cap>
#include <l4/sys/task>

static Dbg dbg(Dbg::Server, "svr");


Itas_br_manager::Itas_br_manager()
{
  for (auto &c : rcv_cap_slots)
    c = Global::cap_alloc->alloc<void>();
}

void Itas_br_manager::setup_wait(l4_utcb_t *utcb, L4::Ipc_svr::Reply_mode)
{
  unsigned i = 0;
  for (auto &c : rcv_cap_slots)
    l4_utcb_br_u(utcb)->br[i++] = L4::Ipc::Small_buf(c.cap(),
                                                     L4_RCV_ITEM_LOCAL_ID).raw();
  l4_utcb_br_u(utcb)->br[i] = 0;
  l4_utcb_br_u(utcb)->bdr = 0;
}

int Itas_br_manager::alloc_buffer_demand(L4::Type_info::Demand const &demand)
{
  if (demand.caps > Num_receive_buffers
      || demand.ports != 0
      || demand.mem != 0
      || demand.flags != 0)
    return -L4_EINVAL;

  return 0;
}

L4::Cap<void> Itas_br_manager::get_rcv_cap(int index) const
{
  if (index >= 0 && static_cast<unsigned>(index) < cxx::array_size(rcv_cap_slots))
    return rcv_cap_slots[index];
  else
    return L4::Cap<void>::Invalid;
}

int Itas_br_manager::realloc_rcv_cap(int)
{ return -L4_ENOMEM; }

int Itas_br_manager::add_timeout(L4::Ipc_svr::Timeout *, l4_kernel_clock_t)
{ return -L4_ENOSYS; }

int Itas_br_manager::remove_timeout(L4::Ipc_svr::Timeout *)
{ return -L4_ENOSYS; }


Server server;


L4::Cap<void> Dispatcher::register_obj(L4::Epiface *o)
{
  int err = server.alloc_buffer_demand(o->get_buffer_demand());
  if (err < 0)
    return L4::Cap<void>(err | L4_INVALID_CAP_BIT);

  auto cap = L4Re::make_unique_cap<L4::Kobject>(Global::cap_alloc);
  if (!cap)
    return cap.get();

  l4_umword_t id = reinterpret_cast<l4_umword_t>(o);
  err = l4_error(_factory->create_gate(cap.get(), _thread, id));
  if (err < 0)
    return L4::Cap<void>(err | L4_INVALID_CAP_BIT);

  err = o->set_server(&server, cap.get(), true);
  if (err < 0)
    return L4::Cap<void>(err | L4_INVALID_CAP_BIT);

  return cap.release();
}

void Dispatcher::unregister_obj(L4::Epiface *o)
{
  L4::Epiface::Stored_cap c;

  if (!o || !o->obj_cap().is_valid())
    return;

  c = o->obj_cap();

  L4::Cap<L4::Task>(L4Re::This_task)
    ->unmap(c.fpage(), L4_FP_ALL_SPACES | L4_FP_DELETE_OBJ);

  // make sure unhandled ipc ends up with the null handler
  L4::Thread::Modify_senders todo;
  todo.add(~3UL, reinterpret_cast<l4_umword_t>(o),
           ~0UL, reinterpret_cast<l4_umword_t>
                 (static_cast<L4::Epiface *>(&_null_handler)));
  _thread->modify_senders(todo);

  // we use bit 4 to indicated an internally allocated cap
  if (c.managed())
    Global::cap_alloc->free(c);

  o->set_server(0, L4::Cap<void>::Invalid);
}

l4_msgtag_t
Dispatcher::dispatch(l4_msgtag_t t, l4_umword_t obj, l4_utcb_t *utcb)
{
  dbg.printf("request: tag=0x%lx proto=%ld obj=0x%lx\n", t.raw, t.label(), obj);
  return L4::Basic_registry::dispatch(t, obj, utcb);
}

Dispatcher dispatcher;
