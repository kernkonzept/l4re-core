/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <stdio.h>
#include <string.h>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/re/dataspace>
#include <l4/re/rm>
#include <l4/re/env>
#include <l4/sys/kip>

#include "dispatcher.h"
#include "debug.h"
#include "globals.h"
#include "region.h"

using L4Re::Rm;
using L4Re::Dataspace;
using L4Re::Util::Region;

Region_map::Region_map()
  : Base(0,0)
{}

void
Region_map::init()
{
  // Only get virtual address space limits. Reserved regions are gathered from
  // remote region manager and would collide otherwise.
  for (auto const &m: L4::Kip::Mem_desc::all(l4re_kip()))
    if (m.is_virtual() && m.type() == L4::Kip::Mem_desc::Conventional)
      set_limits(m.start(), m.end());
}


l4_ret_t
Region_handler::map(l4_addr_t local_addr, Region const &r, bool writable,
                    l4_umword_t *result) const noexcept
{
  *result = 0;

  auto r_flags = _flags;
  if (!writable)
    r_flags -= L4Re::Rm::F::W;

  if ((r_flags & (Rm::F::Reserved | Rm::F::Kernel)) || !_mem.is_valid())
    return -L4_ENOENT;

  if (r_flags & Rm::F::Pager)
    {
      L4::Ipc::Snd_fpage rfp;
      return l4_error(L4::cap_reinterpret_cast<L4::Pager>(_mem)
                      ->page_fault(local_addr, -3UL,
                                   L4::Ipc::Rcv_fpage::mem(0, L4_WHOLE_ADDRESS_SPACE),
                                   rfp));
    }
  else
    {
      // align to 16byte, some DS implementations are too picky about
      // possible r/w etc. bits in the offset
      local_addr &= ~0x0fUL;
      l4_addr_t offset = local_addr - r.start() + _offs;
      L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(_mem);
      L4Re::Dataspace::Flags flags = map_flags(r_flags);
      return ds->map(offset, flags, local_addr, r.start(), r.end());
    }
}

void
Region_handler::free(l4_addr_t start, unsigned long size) const noexcept
{
  if (_flags & (Rm::F::Reserved | Rm::F::Kernel | Rm::F::Pager))
    return;

  if (!_mem.is_valid())
    return;

  L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(_mem);
  ds->clear(offset() + start, size);
}

l4_ret_t
Region_handler::map_info(l4_addr_t *start_addr, l4_addr_t *end_addr) const noexcept
{
  if (!_mem)
    return 0;

  if (_flags & (Rm::F::Pager | Rm::F::Reserved | Rm::F::Kernel))
    return 0;

  return _mem->map_info(start_addr, end_addr);
}

bool
Region_handler::attached(l4_addr_t beg, l4_addr_t end) noexcept
{
  // Stack regions must be attached with the moe region manager. The page fault
  // trampoline is executing on the stack and thus must be pagable via
  // page fault IPC!
  //
  // Unfortunately, we don't have the information here if the dataspace is used
  // as stack memory. To be on the safe side, attach all regions that are
  // backed by a real dataspace.
  if (!_mem || _flags & (Rm::F::Pager | Rm::F::Kernel | Rm::F::Reserved))
    return true;

  if (_attached)
    return true;

  auto moe_rm = L4Re::Env::env()->rm();
  // Moe must not free the dataspace region on detach.
  auto flags = _flags & ~Rm::F::Detach_free;
  l4_ret_t res = moe_rm->attach(&beg, end - beg + 1, flags,
                                L4::Ipc::make_cap_rw(_mem), _offs);
  if (res < 0)
    {
      // If this was not a moe dataspace, we'll get an L4_ENOENT error. This is
      // fine because such a dataspace is not usable as stack space.
      //
      // Any other error is more serious, though. This means that there is an
      // unexpected inconsistency between the actual remote region map and our
      // model of it. Something running in the l4re_itas must have attached a
      // memory region there, which is a bug, and must be fixed.
      if (res != -L4_ENOENT)
        {
          Err(Err::Fatal)
            .printf("%s: Could not attach DS [0x%lx..0x%lx] to remote region map (%d).\n",
                    Global::l4re_aux->binary, beg, end, res);
          return false;
        }
    }
  else
    _attached = true;

  return true;
}

void
Region_handler::detached(l4_addr_t beg, l4_addr_t end) const noexcept
{
  if (!_mem || _flags & (Rm::F::Pager | Rm::F::Kernel | Rm::F::Reserved))
    return;

  if (!_attached)
    return;

  auto moe_rm = L4Re::Env::env()->rm();
  l4_ret_t res = moe_rm->detach(beg, end - beg + 1, nullptr,
                                L4::Cap<L4::Task>::Invalid);
  if (res < 0)
    {
      Dbg warn(Dbg::Warn);
      warn.printf("%s: Could not detach DS [0x%lx..0x%lx] from remote region map (%d)\n",
                  Global::l4re_aux->binary, beg, end, res);
    }
}

void
Region_map::debug_dump(unsigned long /*function*/) const
{
  printf("Region mapping: limits [%lx-%lx]\n", min_addr(), max_addr());
  printf(" Area map:\n");
  for (Region_map::Const_iterator i = area_begin(); i != area_end(); ++i)
    printf("  [%10lx-%10lx] -> flags=%x\n",
           i->first.start(), i->first.end(), i->second.flags());

  printf(" Region map:\n");
  for (Region_map::Const_iterator i = begin(); i != end(); ++i)
    {
      unsigned f = i->second.flags();
      char r[8];
      r[0] = f & L4Re::Rm::F::R ? 'r' : '-';
      r[1] = f & L4Re::Rm::F::W ? 'w' : '-';
      r[2] = f & L4Re::Rm::F::X ? 'x' : '-';
      r[3] = f & L4Re::Rm::F::Kernel      ? 'K' : '-';
      r[4] = f & L4Re::Rm::F::Detach_free ? 'D' : '-';
      r[5] = f & L4Re::Rm::F::Pager       ? 'P' : '-';
      r[6] = f & L4Re::Rm::F::Reserved    ? 'R' : '-';
      r[7] = '\0';
      printf("  %010lx-%010lx ds=%04lx@%07llx %s/%04x %07llx: %.*s \n",
             i->first.start(), i->first.end(),
             i->second.memory().cap() >> L4_CAP_SHIFT, i->second.offset(),
             r, i->second.flags(),
             i->first.backing_offset(), i->first.name_len(), i->first.name());
    }
}

l4_ret_t
Region_map_svr::op_exception(L4::Exception::Rights, l4_exc_regs_t &u,
                             L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Dbg w(Dbg::Warn);
  w.printf("%s: Unhandled exception: PC=0x%lx PFA=0x%lx LdrFlgs=0x%lx\n",
           Global::l4re_aux->binary, l4_utcb_exc_pc(&u), l4_utcb_exc_pfa(&u),
           Global::l4re_aux->ldr_flags);

  return -L4_ENOREPLY;
}

l4_ret_t
Region_map_svr::op_io_page_fault(L4::Io_pager::Rights,
                                 l4_fpage_t io_pfa, l4_umword_t pc,
                                 L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  Err().printf("IO-port-fault: port=0x%lx size=%d pc=0x%lx\n",
               l4_fpage_ioport(io_pfa), 1 << l4_fpage_size(io_pfa), pc);
  return -L4_ENOMEM;
}

/**
 * Thread page fault trampoline.
 *
 * Try to resolve the page fault of the current thread.
 *
 * \attention This function is called in the context of the faulting application
 *            thread and is using its stack.
 */
L4UTIL_THREAD_CXX_FUNC_INTERRUPT_IMPL(Region_map_svr, thread_pf_handler,
                                      l4_pf_trampoline_t *tramp, l4_addr_t addr)
{
  // We need to save everything which could be modified by IPC while
  // op_page_fault() is executed:
  // - all MRs
  constexpr size_t num_mr_regs = L4_UTCB_GENERIC_DATA_SIZE;
  // - all BRs and the BDR
  constexpr size_t num_br_regs = L4_UTCB_GENERIC_BUFFERS_SIZE + 1 /* bdr */;
  // - the IPC error field.
  // Assert that the IPC error field comes immediately after the last BR.
  static_assert(L4_UTCB_BUF_REGS_OFFSET + num_br_regs * sizeof(long)
                == L4_UTCB_THREAD_REGS_OFFSET);
  static_assert(offsetof(l4_thread_regs_t, error) == 0);
  long utcb_save[num_mr_regs + num_br_regs + 1 /* error */];

  memcpy(utcb_save, l4_utcb_mr(), num_mr_regs * sizeof(long));
  memcpy(utcb_save + num_mr_regs, l4_utcb_br(), (num_br_regs + 1) * sizeof(long));

  L4::Ipc::Opt<L4::Ipc::Snd_fpage> fp;
  long res = Global::local_rm->op_page_fault(L4::Pager::Rights(0), addr,
                                             tramp->state.ip, fp);

  memcpy(l4_utcb_mr(), utcb_save, num_mr_regs * sizeof(long));
  memcpy(l4_utcb_br(), utcb_save + num_mr_regs, (num_br_regs + 1) * sizeof(long));

  L4::Cap<L4::Thread> this_thread;
  if (L4_LIKELY(res >= 0))
    this_thread->pf_trampoline_resume();
  else
    this_thread->pf_trampoline_reflect();

  __builtin_unreachable();
}
