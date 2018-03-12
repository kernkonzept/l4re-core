/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "boot_fs.h"
#include "loader.h"
#include "globals.h"
#include "dataspace_anon.h"
#include "dataspace_noncont.h"
#include "debug.h"
#include "exception.h"
#include "app_task.h"
#include "name_space.h"
#include "dataspace_util.h"
#include "region.h"

#include <l4/cxx/auto_ptr>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/cxx/exceptions>
#include <l4/cxx/ipc_helper>

#include <l4/sys/scheduler>
#include <l4/sys/utcb.h>
#include <l4/sys/kernel_object.h>
#include <l4/sys/factory>
#include <l4/sys/task>
#include <l4/sys/thread>
#include <l4/re/error_helper>

#include <l4/re/env>

#include <l4/util/bitops.h>

#include <cctype>
#include "args.h"


using L4::Cap;
using L4Re::chksys;

using Moe::Stack;
using Moe::Boot_fs;

enum
{
  Stack_size        = 0x8000,
  Stack_address     = 0xb0000000,
};


static
Moe::Dataspace *
__alloc_app_stack(Allocator *a, Moe::Stack *_stack, unsigned long size)
{
  cxx::Auto_ptr<Moe::Dataspace> stack(a->alloc(size));

  _stack->set_local_top(stack->address(size - L4_PAGESIZE).adr<char*>() + L4_PAGESIZE);
  return stack.release();
}

bool Loader::start(cxx::String const &init_prog, cxx::String const &cmdline)
{
  Dbg info(Dbg::Info);
  Dbg ldr(Dbg::Loader);

  info.printf("Starting: %.*s %.*s\n",
              init_prog.len(), init_prog.start(), cmdline.len(), cmdline.start());

  return exec(init_prog, cmdline);
}


class Ldr_task : public App_task
{
public:

private:
  l4_umword_t _entry, _stack_ptr;

public:
  Ldr_task()
  : _entry(0), _stack_ptr(0)
  {}

  void set_entry_data(l4_umword_t entry, l4_umword_t stack_ptr)
  { _stack_ptr = stack_ptr; _entry = entry; }
};

Moe_app_model::Dataspace
Moe_app_model::alloc_ds(unsigned long size) const
{
  Dataspace mem =_task->allocator()->alloc(size);
  if (!mem)
    chksys(-L4_ENOMEM, "ELF loader could not allocate memory");
  return mem;
}

l4_cap_idx_t Moe_app_model::push_initial_caps(l4_cap_idx_t s)
{
  for (auto const &i : *root_name_space())
    _stack.push(L4Re::Env::Cap_entry(i.name().start(),
                                     get_initial_cap(i.name().start(), &s)));

  return s;
}

void Moe_app_model::map_initial_caps(L4::Cap<L4::Task> t, l4_cap_idx_t s)
{
  for (auto const &i : *root_name_space())
    {
      auto c = get_initial_cap(i.name().start(), &s);
      chksys(t->map(L4Re::This_task, i.cap().fpage(L4_CAP_FPAGE_RWS),
                    L4::Cap<void>(c).snd_base()));
    }
}

Moe_app_model::Const_dataspace
Moe_app_model::open_file(char const *name)
{
  Moe::Dataspace const *f = Boot_fs::open_file(name);
  if (!f)
    chksys(-L4_ENOENT, name);
  return f;
}

void
Moe_app_model::prog_attach_ds(l4_addr_t addr, unsigned long size,
                              Const_dataspace ds, unsigned long offset,
                              unsigned flags, char const *what)
{
  void *x = _task->rm()->attach((void*)addr, size,
                                Region_handler(ds, L4_INVALID_CAP,
                                               offset, flags),
                                flags);
  if (x == L4_INVALID_PTR)
    chksys(-L4_ENOMEM, what);
}

int
Moe_app_model::prog_reserve_area(l4_addr_t *start, unsigned long size,
                                 unsigned flags, unsigned char align)
{
  l4_addr_t a = _task->rm()->attach_area(*start, size, flags, align);
  if (a == L4_INVALID_ADDR)
    return -L4_ENOMEM;

  *start = a;
  return 0;
}

bool
Loader::exec(cxx::String const &prog, cxx::String const &args)
{
  static auto *_init_task = Moe::Moe_alloc::allocator()->make_obj<Ldr_task>();

  launch(_init_task, prog, args);
  return 0;
}


void
Moe_app_model::copy_ds(Dataspace dst, unsigned long dst_offs,
                       Const_dataspace src, unsigned long src_offs,
                       unsigned long size)
{
  Dataspace_util::copy(dst, dst_offs, src, src_offs, size);
}


l4_addr_t
Moe_app_model::local_attach_ds(Const_dataspace ds, unsigned long /*size*/,
                               unsigned long offset) const
{
  return (l4_addr_t)ds->address(offset).adr();
}

void
Moe_app_model::local_detach_ds(l4_addr_t /*addr*/, unsigned long /*size*/) const
{
}


Moe_app_model::Moe_app_model(App_task *t, cxx::String const &prog,
                             cxx::String const &args)
: _task(t), _prog(prog), _args(args)
{
  enum
  {
#ifdef ARCH_mips
    Utcb_area_start        = 0x73000000, // this needs to be lower on MIPS
#else
    Utcb_area_start        = 0xb3000000,
#endif
    Default_max_threads    = 16,
  };
  // set default values for utcb area, values may be changed by loader
  _info.utcbs_start = Utcb_area_start;
  _info.utcbs_log2size  = l4util_log2(Default_max_threads * L4_UTCB_OFFSET);

  // pimp the UTCB area at least to L4_PAGESIZE
  if (_info.utcbs_log2size < L4_PAGESHIFT)
    _info.utcbs_log2size  = L4_PAGESHIFT;

  extern char __L4_KIP_ADDR__[];
  // set default values for the application stack
  _info.kip = (l4_addr_t)__L4_KIP_ADDR__;

}


Moe_app_model::Dataspace
Moe_app_model::alloc_app_stack()
{
  return __alloc_app_stack(_task->allocator(), &_stack, _stack.stack_size());
}

void
Moe_app_model::init_prog()
{
  argv.al = argv.a0 = _stack.push_str(_prog.start(), _prog.len());

  for (cxx::Pair<cxx::String, cxx::String> a = next_arg(_args);
       !a.first.empty(); a = next_arg(a.second))
    argv.al = _stack.push_str(a.first.start(), a.first.len());

  envp.a0 = envp.al = 0;

  Dbg info(Dbg::Info);

  Allocator *allocator = Allocator::root_allocator();
  _info.mem_alloc = allocator->obj_cap().fpage();
  _info.log = L4Re::Env::env()->log().fpage();
  _info.factory = L4Re::Env::env()->factory().fpage();
  _info.scheduler = L4Re::Env::env()->scheduler().fpage();

  _info.ldr_flags = Moe::ldr_flags;
  _info.l4re_dbg = Moe::l4re_dbg;

  info.printf("loading '%s'\n", argv.a0);
}

void
Moe_app_model::get_task_caps(L4::Cap<L4::Factory> *factory,
                             L4::Cap<L4::Task> *task,
                             L4::Cap<L4::Thread> *thread)
{
  object_pool.cap_alloc()->alloc(_task);
  _task->task_cap(object_pool.cap_alloc()->alloc<L4::Task>());
  _task->thread_cap(object_pool.cap_alloc()->alloc<L4::Thread>());

  prog_info()->rm = _task->rm()->obj_cap().fpage();
  prog_info()->parent = _task->obj_cap().fpage();

  *task = _task->task_cap();
  *thread = _task->thread_cap();
  *factory = L4Re::Env::env()->factory();
}

