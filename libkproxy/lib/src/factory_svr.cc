// vi:ft=cpp
/**
 * \internal
 * \file
 * \brief
 */
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
#include <l4/libkproxy/factory_svr>
#include <l4/cxx/ipc_stream>

namespace L4kproxy {

class Factory_hndl
{
public:
  static int handle_factory(Factory_svr *svr, Factory_interface *fi,
                            L4::Ipc::Iostream &ios)
    {
      unsigned long limit;
      L4::Cap<L4::Factory> f = svr->cap_alloc<L4::Factory>();
      if (!f.is_valid())
        return -L4_ENOMEM;
      ios >> limit;
      int r = fi->create_factory(f, limit);
      if (r == 0)
        ios << f;
      return r;
    }

  static int handle_task(Factory_svr *svr, Factory_interface *fi,
                         L4::Ipc::Iostream &ios)
    {
      l4_fpage_t utcb_area;
      L4::Cap<L4::Task> t = svr->cap_alloc<L4::Task>();
      if (!t.is_valid())
        return -L4_ENOMEM;
      ios >> utcb_area.raw;
      int r = fi->create_task(t, utcb_area);
      if (r == 0)
        ios << t;
      return r;
    }

  static int handle_thread(Factory_svr *svr, Factory_interface *fi,
                           L4::Ipc::Iostream &ios)
    {
      L4::Cap<L4::Thread> t = svr->cap_alloc<L4::Thread>();
      if (!t.is_valid())
        return -L4_ENOMEM;
      int r = fi->create_thread(t);
      if (r == 0)
        ios << t;
      return r;
    }

  static int handle_gate(Factory_svr *svr, Factory_interface *fi,
                         L4::Ipc::Iostream &ios)
    {
      l4_umword_t label;
      L4::Ipc::Snd_fpage f;
      L4::Cap<void> g = svr->cap_alloc<void>();
      if (!g.is_valid())
        return -L4_ENOMEM;

      ios >> label >> f;

      int r = fi->create_gate(g, svr->received_thread(f), label);

      if (r == 0)
        ios << g;
      return r;
    }

  static int handle_irq(Factory_svr *svr, Factory_interface *fi,
                        L4::Ipc::Iostream &ios)
    {
      L4::Cap<L4::Irq> i = svr->cap_alloc<L4::Irq>();
      if (!i.is_valid())
        return -L4_ENOMEM;
      int r = fi->create_irq(i);
      if (r == 0)
        ios << i;
      return r;
    }

  static int handle_vm(Factory_svr *svr, Factory_interface *fi,
                       L4::Ipc::Iostream &ios)
    {
      L4::Cap<L4::Vm> i = svr->cap_alloc<L4::Vm>();
      if (!i.is_valid())
        return -L4_ENOMEM;
      int r = fi->create_vm(i);
      if (r == 0)
        ios << i;
      return r;
    }
};

int Factory_svr::factory_dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{
  L4::Opcode op;
  ios >> op;
  switch (op)
    {
    case L4_PROTO_FACTORY:
      return Factory_hndl::handle_factory(this, _factory, ios);
    case L4_PROTO_THREAD:
      return Factory_hndl::handle_thread(this, _factory, ios);
    case L4_PROTO_TASK:
      return Factory_hndl::handle_task(this, _factory, ios);
    case 0:
      return Factory_hndl::handle_gate(this, _factory, ios);
    case L4_PROTO_IRQ:
      return Factory_hndl::handle_irq(this, _factory, ios);
    case L4_PROTO_VM:
      return Factory_hndl::handle_vm(this, _factory, ios);
    default:
      return -L4_ENOSYS;
    };
}

}
