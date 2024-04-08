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
#include "loader.h"
#include "region.h"
#include "debug.h"
#include "globals.h"
#include "mem_layout.h"
#include "switch_stack.h"

#include <l4/bid_config.h>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/sys/types.h>
#include <l4/sys/factory>
#include <l4/sys/scheduler>
#include <l4/sys/thread>

#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/re/mem_alloc>
#include <l4/re/env>
#include <l4/re/util/env_ns>
#include <l4/re/l4aux.h>
#include <l4/re/error_helper>
#include <l4/sys/debugger.h>
#include <l4/util/util.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define L4RE_USE_LOCAL_PAGER_GATE 1

using L4Re::Mem_alloc;
using L4Re::Dataspace;
using L4Re::Rm;
using L4::Cap;
using L4::Thread;
using L4Re::Env;
using L4Re::chksys;

namespace {

struct Entry_data
{
  l4_addr_t entry;
  l4_addr_t stack;
  l4_umword_t ex_regs_flags;
  L4::Cap<L4Re::Rm> pager;
};

static Loader *__loader;
static Cap<Dataspace> __binary;
static Cap<Dataspace> __loader_stack;
static void *__loader_stack_p;
static Entry_data __loader_entry;
static Region_map *__rm;
static Cap<Thread> app_thread;

static
void unmap_stack_and_start()
{
  L4Re::Env::env()->rm()->detach(l4_addr_t(__loader_stack_p) - 1, 0);
  Global::cap_alloc->free(__loader_stack);
  L4::Cap<Thread> self;
  if (__loader_entry.ex_regs_flags)
    chksys(self->ex_regs(~0UL, ~0UL, __loader_entry.ex_regs_flags),
           "l4re_kernel: Change mode according to L4RE_ELF_AUX_T_EX_REGS_FLAGS.");
  switch_stack(__loader_entry.stack,
               reinterpret_cast<void(*)()>(__loader_entry.entry));
}

}

L4Re_app_model::L4Re_app_model(L4::Cap<L4Re::Rm> rm, void *)
: _rm(rm)
{
  _info.kip = (l4_addr_t)l4re_kip();
}

L4Re_app_model::Dataspace
L4Re_app_model::alloc_ds(unsigned long size) const
{ return alloc_ds(size, 0, 0); }

L4Re_app_model::Dataspace
L4Re_app_model::alloc_ds(unsigned long size, l4_addr_t paddr) const
{ return alloc_ds(size, paddr, L4Re::Mem_alloc::Fixed_paddr); }

L4Re_app_model::Dataspace
L4Re_app_model::alloc_ds(unsigned long size, l4_addr_t paddr,
                         unsigned long flags) const
{
  Dataspace mem = chkcap(Global::cap_alloc->alloc<L4Re::Dataspace>(),
      "l4re_kernel: ELF loader: could not allocate capability");
  if (Global::l4re_aux->ldr_flags & L4RE_AUX_LDR_FLAG_PINNED_SEGS)
    flags |= L4Re::Mem_alloc::Pinned;
  chksys(Global::allocator->alloc(size, mem, flags, 0, paddr),
         "l4re_kernel: Loading writable ELF segment.");
  return mem;
}

L4Re_app_model::Const_dataspace
L4Re_app_model::open_file(char const *name)
{
  using L4Re::chkcap;
  L4Re::Util::Env_ns ens(L4Re::Env::env(), L4Re::Cap_alloc::get_cap_alloc(*Global::cap_alloc));

  L4::Cap<L4Re::Dataspace> file;

  file = chkcap(ens.query<L4Re::Dataspace>(name), name, 0);

  return file;
}

void
L4Re_app_model::prog_attach_ds(l4_addr_t addr, unsigned long size,
                               Const_dataspace ds, unsigned long offset,
                               L4Re::Rm::Flags flags, char const *what)
{
  if (Global::l4re_aux->ldr_flags & L4RE_AUX_LDR_FLAG_EAGER_MAP)
    flags |= L4Re::Rm::F::Eager_map;

  chksys(_rm->attach(&addr, size, flags,
                     L4::Ipc::make_cap(ds, flags.cap_rights()),
                     offset), what);
}

void
L4Re_app_model::copy_ds(Dataspace dst, unsigned long dst_offs,
                        Const_dataspace src, unsigned long src_offs,
                        unsigned long size)
{
  L4Re::chksys(dst->copy_in(dst_offs, src, src_offs, size),
               "l4re_kernel: Copy-in failed.");
}

l4_addr_t
L4Re_app_model::local_attach_ds(Const_dataspace ds, unsigned long size,
                                unsigned long offset) const
{
  l4_addr_t pg_offset = l4_trunc_page(offset);
  l4_addr_t in_pg_offset = offset - pg_offset;
  unsigned long pg_size = l4_round_page(size + in_pg_offset);
  l4_addr_t vaddr = 0;
  chksys(_rm->attach(&vaddr, pg_size, L4Re::Rm::F::Search_addr | L4Re::Rm::F::R,
                     ds, pg_offset),
         "l4re_kernel: ELF loader: attach temporary VMA");
  return vaddr + in_pg_offset;
}

void
L4Re_app_model::local_detach_ds(l4_addr_t addr, unsigned long /*size*/) const
{
  l4_addr_t pg_addr = l4_trunc_page(addr);
  chksys(_rm->detach(pg_addr, 0), "l4re_kernel: ELF loader: detach temporary VMA");
}

int
L4Re_app_model::prog_reserve_area(l4_addr_t *start, unsigned long size,
                                  L4Re::Rm::Flags flags, unsigned char align)
{
  return _rm->reserve_area(start, size, flags, align);
}

L4Re_app_model::Dataspace
L4Re_app_model::alloc_app_stack()
{
  // Allocate the stack for the application
  L4::Cap<L4Re::Dataspace> stack
    = chkcap(Global::cap_alloc->alloc<L4Re::Dataspace>(),
      "l4re_kernel: ELF loader: could not allocate capability");

  chksys(Global::allocator->alloc(_stack.stack_size(), stack));

  L4Re::Rm::Flags flags(Rm::F::Search_addr | Rm::F::RW);
  if (Global::l4re_aux->ldr_flags & L4RE_AUX_LDR_FLAG_EAGER_MAP)
    flags |= L4Re::Rm::F::Eager_map;

#ifdef CONFIG_MMU
  void *_s = reinterpret_cast<void*>(_stack.target_addr());
#else
  void *_s = nullptr;
#endif
  chksys(_rm->attach(&_s, _stack.stack_size(), flags,
                     L4::Ipc::make_cap_rw(stack), 0),
         "l4re_kernel: Attach application stack.");
  _stack.set_target_stack(l4_addr_t(_s), _stack.stack_size());
  _stack.set_local_addr(l4_addr_t(_s));
  return stack;
}

void
L4Re_app_model::extra_elf_auxv()
{
}

void
L4Re_app_model::push_envp()
{
  _stack.push(l4_umword_t(0));
  for (char const *const *e = Global::envp; *e; ++e)
    _stack.push(*e);
}

void
L4Re_app_model::push_argv()
{
  _stack.push(l4_umword_t(0));
  for (int i = Global::argc - 1; i >= 0; --i)
    _stack.push(Global::argv[i]);

  // ARGC
  _stack.push(l4_umword_t(Global::argc));
}

bool
L4Re_app_model::all_segs_cow()
{
  return Global::l4re_aux->ldr_flags & L4RE_AUX_LDR_FLAG_ALL_SEGS_COW;
}

L4Re::Env *
L4Re_app_model::add_env()
{
  L4Re::Env *e = const_cast<L4Re::Env *>(L4Re::Env::env());

  e->rm(__loader_entry.pager);
  e->main_thread(app_thread);
  return e;

}

void
L4Re_app_model::start_prog(L4Re::Env const *)
{
  __loader_entry.stack = l4_addr_t(_stack.ptr());
  __loader_entry.entry = _info.entry;
  __loader_entry.ex_regs_flags = _info.ex_regs_flags;
  switch_stack(__loader_entry.stack, &unmap_stack_and_start);
}

enum
{
  Loader_stack_size = 8 * 1024,
};

#ifdef L4_LOADER_USE_ASM_STUB
extern "C" void loader_thread(void);
extern "C" void loader_thread_c(void);
void loader_thread_c(void)
#else
static void
loader_thread()
#endif
{
  if (!__loader->launch(__binary, __loader_entry.pager))
    {
      Err(Err::Fatal).printf("Could not load binary '%s'.\n",
                             Global::l4re_aux->binary);
      exit(1);
    }
}

bool Loader::start(Cap<Dataspace> bin, Region_map *rm, l4re_aux_t *aux)
{
  __loader_stack = Global::cap_alloc->alloc<Dataspace>();
  Global::allocator->alloc(Loader_stack_size, __loader_stack);

  if (!__loader_stack.is_valid())
    {
      Err(Err::Fatal).printf("Could not allocate loader stack (%s).\n",
                             Global::l4re_aux->binary);
      return false;
    }

  void *vma_start
#ifdef CONFIG_MMU
    = reinterpret_cast<void *>(Mem_layout::Loader_vma_start);
#else
    = nullptr;
#endif
  __loader_stack_p
    = Global::local_rm->attach(vma_start, Loader_stack_size,
                               Region_handler(__loader_stack,
                                              __loader_stack.cap(),
                                              0, L4Re::Rm::F::RW),
                               L4Re::Rm::F::Search_addr);

  if (__loader_stack_p == L4_INVALID_PTR)
    {
      Err(Err::Fatal).printf("Could not attach loader stack (%s).\n",
                             Global::l4re_aux->binary);
      return false;
    }

  long ret
    = L4Re::Env::env()->rm()->attach(&__loader_stack_p, Loader_stack_size,
                                     L4Re::Rm::F::RW,
                                     L4::Ipc::make_cap_rw(__loader_stack), 0);
  if (ret)
    {
      // The loader stack is already attached to the local region map. We
      // tried to attach it to the remote region map (e.g. moe) as well, and
      // this failed. This means that there is an unexpected inconsistency
      // between the actual remote region map and our model of it. Something
      // running in the l4re_kernel must have attached a memory region there,
      // which is a bug, and must be fixed.
      Err(Err::Fatal)
        .printf("Could not attach loader stack to remote region map (%ld, %s).\n",
                ret, Global::l4re_aux->binary);
      return false;
    }

  char *stack_addr = reinterpret_cast<char *>(__loader_stack_p)
                   + Loader_stack_size;
  l4_umword_t *sp = reinterpret_cast<l4_umword_t *>(stack_addr);

  *(--sp) = 0;

  __loader_stack_p = sp;


  __loader = this;
  __rm = rm ;
  __binary = bin;

  if (0)
    L4::cout << "l4re: start file " << bin << " entry="
             << reinterpret_cast<void *>(__loader_entry.entry) << '\n';

  Env *const env = const_cast<Env *>(Env::env());

  app_thread = Cap<Thread>(env->first_free_cap() << L4_CAP_SHIFT);
  env->first_free_cap((app_thread.cap() >> L4_CAP_SHIFT)+1);
#ifdef L4RE_USE_LOCAL_PAGER_GATE
  __loader_entry.pager = Global::cap_alloc.alloc<Rm>();
  chksys(env->factory()->create_gate(__loader_entry.pager, env->main_thread(), 0),
         "l4re_kernel: Create pager gate.");
#else
  __loader_entry.pager = L4::cap_reinterpret_cast<Rm>(env->main_thread());
#endif

  chksys(env->factory()->create(app_thread), "l4re_kernel: Create app thread.");

  l4_debugger_set_object_name(app_thread.cap(),
                              strrchr(aux->binary, '/')
                                ? strrchr(aux->binary, '/') + 1 : aux->binary);

  Thread::Attr attr;
  attr.pager(__loader_entry.pager);
  attr.exc_handler(__loader_entry.pager);
  attr.bind(reinterpret_cast<l4_utcb_t*>(env->first_free_utcb()),
            L4Re::This_task);

  env->first_free_utcb(env->first_free_utcb() + L4_UTCB_OFFSET);

  chksys(app_thread->control(attr), "l4re_kernel: Setup app thread.");
  chksys(env->scheduler()->run_thread(app_thread,
                                      l4_sched_param(L4RE_MAIN_THREAD_PRIO)),
         "l4re_kernel: Set app priority.");
  unsigned long stack = reinterpret_cast<unsigned long>(__loader_stack_p);
  chksys(app_thread->ex_regs(reinterpret_cast<unsigned long>(&loader_thread),
                             l4_align_stack_for_direct_fncall(stack), 0),
                             "l4re_kernel: Start app thread.");

  return true;
}
