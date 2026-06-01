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
#include <l4/cxx/result>
#include <l4/re/error_helper>
#include <l4/re/dataspace>
#include <l4/re/rm>
#include <l4/re/env>
#include <l4/sys/kip>

#include "dispatcher.h"
#include "debug.h"
#include "globals.h"
#include "region.h"
#include "safe_memcpy.h"

using L4Re::Rm;
using L4Re::Dataspace;
using L4Re::Util::Region;


namespace {

/**
 * Unmap a range of pages.
 *
 * \pre `start` and `size` must be page aligned.
 */
void unmap_page_range(l4_addr_t start, l4_addr_t size) noexcept
{
  unsigned order = L4_LOG2_PAGESIZE;
  l4_addr_t fp_size = (1UL << order);
  for (l4_addr_t p = start; size; p += fp_size, size -= fp_size)
    {
      while (fp_size > size)
        {
          --order;
          fp_size >>= 1;
        }

      for (;;)
        {
          unsigned long m = fp_size << 1;
          if (m > size)
            break;

          if (p & (m - 1))
            break;

          ++order;
          fp_size <<= 1;
        }

      L4::Cap<L4::Task> task(L4Re::This_task);
      task->unmap(l4_fpage(p, order, L4_FPAGE_RWX),
                  L4_FP_ALL_SPACES);
    }
}

#ifdef CONFIG_MMU
l4_addr_t cow_area;
Mutex cow_lock;
#endif

}


Page_bitmap::Page_bitmap(unsigned num_bits) noexcept
: Bitmap_base(&_storage)
{
  memset(bit_buffer(), 0, bit_buffer_bytes(num_bits));
}


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


int
Region_handler::init(Region_map *rm, unsigned long size) noexcept
{
#ifdef CONFIG_MMU
  // On MMU systems, only private, DS backed mappings require a shadow
  // anonymous DS. Shared regions are mapped directly and anonymous regions are
  // completely handled by the moe region manager.
  if ((_flags & (Rm::F::Private | Rm::F::Anonymous)) != Rm::F::Private)
    return 0;

  // TODO: superpage support
  auto alloc = rm->alloc_anon_mem(size);
  if (!alloc)
    return alloc.error();

  _anon_mem = cxx::move(alloc.result().ds);
  _anon_pages = cxx::move(alloc.result().page_bm);
  _anon_offs = alloc.result().offs;

  return 0;
#else
  // Everything except shared, DS backed mappings require a (shadow) anonymous
  // DS on no-MMU systems.
  if ((_flags & (Rm::F::Private | Rm::F::Anonymous)) == 0)
    return 0;

  auto alloc = rm->alloc_anon_mem(size);
  if (!alloc)
    return alloc.error();

  _anon_mem = cxx::move(alloc.result().ds);
  _anon_offs = alloc.result().offs;

  if (!(_flags & Rm::F::Anonymous))
    // TODO: implement map+copy if copy_in fails...
    if (l4_ret_t err = _anon_mem->copy_in(_anon_offs, _mem.itas_cap(),
                                          l4_trunc_page(_offs), size);
        err < 0)
      return err;

  return 0;
#endif
}

#ifdef CONFIG_MMU
l4_ret_t
Region_handler::map_cow_page(Region const &r, L4Re::Dataspace::Flags ds_flags,
                             l4_addr_t addr) const noexcept
{
  L4::Cap<L4Re::Dataspace> ds = _mem.itas_cap();

  // We must map exactly one page. Prevent the dataspace provider from mapping
  // a superpage, potentially squashing adjacent pages.
  addr = l4_trunc_page(addr);
  l4_addr_t start = addr;
  l4_addr_t end = start + L4_PAGESIZE;

  l4_addr_t offset = start - r.start() + _offs;
  l4_addr_t anon_offset = start - r.start() + _anon_offs;

  if (_anon_pages->bit(anon_offset >> L4_PAGESHIFT))
    {
      // Already copied. Just map our private copy (again). Can happen after
      // fork() or on concurrent reads while an up-copy takes place (see
      // below).
      return _anon_mem->map(anon_offset, ds_flags, addr, start, end);
    }
  else if (ds_flags & L4Re::Dataspace::F::W)
    {
      // First write access. Do an up-copy....

      // We need a lock when working on the anonymous page. Another thread
      // might simultaneously write the same page!
      // TODO: use some hashed locking for better parallel performance together
      // with a larger mapping area.
      Mutex_guard guard(cow_lock);

      // Another thread could have simultaneously CoW'ed the page. This must
      // have happened recently while we waited for the lock. Assume that the
      // page is still mapped...
      if (_anon_pages->bit(anon_offset >> L4_PAGESHIFT))
        return L4_EOK;

      // We have to make a temporary mapping first because we cannot copy data
      // in-place. Another thread might simultaneously read from the same page
      // and we must have a consistent state all the time. :'(
      L4Re::Rm::Unique_region<char *> bounce_buf(reinterpret_cast<char *>(cow_area));
      if (int err = L4Re::Env::env()->rm()
            ->attach(&bounce_buf, L4_PAGESIZE,
                     L4Re::Rm::F::RW | L4Re::Rm::F::Eager_map,
                     L4::Ipc::make_cap_rw(_anon_mem.get()), anon_offset);
          err < 0)
        return err;

      do
        {
          auto ro_flags = ds_flags & ~L4Re::Dataspace::F::W;
          l4_ret_t err = ds->map(offset, ro_flags, addr, start, end);
          if (err < 0)
            return err;
        }
      while (!safe_memcpy(bounce_buf.get(), reinterpret_cast<char const *>(start),
                          L4_PAGESIZE));

      // Make sure the page copy is observable before the _anon_pages bit is
      // set. Pairs with the _anon_mem->map() above after testing the bit
      // because system calls imply a full memory barrier.
      __atomic_thread_fence(__ATOMIC_RELEASE);

      // From here on, we'll always use the anonymous page. Set the bit
      // *before* we overmap the page. Other threads that read the page
      // concurrently might temporarily get a page fault because Fiasco does an
      // unmap+map!
      _anon_pages->atomic_set_bit(anon_offset >> L4_PAGESHIFT);

      // Atomically map the RW anon page over the previous RO mapping.
      if (int err = _anon_mem->map(anon_offset, ds_flags, addr, start, end);
          err < 0)
        {
          _anon_pages->atomic_clear_bit(anon_offset >> L4_PAGESHIFT);
          return err;
        }

      return 0;
    }
  else
    // We must only map a single page here. Otherwise, the datspace
    // provider might send us a larger mapping, overwritting existing
    // mappings left and right...
    return ds->map(offset, ds_flags, addr, start, end);
}
#endif

l4_ret_t
Region_handler::map(l4_addr_t local_addr, Region const &r, bool writable,
                    l4_umword_t *result) const noexcept
{
  *result = 0;

  auto r_flags = _flags;
  if (!writable)
    r_flags -= L4Re::Rm::F::W;

  if (r_flags & (Rm::F::Reserved | Rm::F::Kernel))
    return -L4_ENOENT;

  if (r_flags & (Rm::F::Pager | Rm::F::Anonymous))
    {
      L4::Ipc::Snd_fpage rfp;
      return l4_error(L4Re::Env::env()->rm()
                      ->page_fault(local_addr, -3UL,
                                   L4::Ipc::Rcv_fpage::mem(0, L4_WHOLE_ADDRESS_SPACE),
                                   rfp));
    }
  else
    {
      if (!_mem)
        return -L4_ENOENT;

      // align to 16byte, some DS implementations are too picky about
      // possible r/w etc. bits in the offset
      local_addr &= ~0x0fUL;

      L4Re::Dataspace::Flags flags = map_flags(r_flags);
      if (_anon_mem)
        {
#ifdef CONFIG_MMU
          return map_cow_page(r, flags, local_addr);
#else
          l4_addr_t anon_offset = local_addr - r.start() + _anon_offs;
          return _anon_mem->map(anon_offset, flags, local_addr, r.start(), r.end());
#endif
        }
      else
        {
          l4_addr_t offset = local_addr - r.start() + _offs;
          auto ds = _mem.itas_cap();
          return ds->map(offset, flags, local_addr, r.start(), r.end());
        }
    }
}

l4_ret_t
Region_handler::page_in(L4Re::Util::Region const &r, l4_addr_t start,
                        l4_addr_t end, L4Re::Rm::Region_flags rights,
                        l4_umword_t *) const noexcept
{
  if (_flags & (L4Re::Rm::F::Kernel | L4Re::Rm::F::Reserved))
    return -L4_ENODEV;

  if (_flags & (Rm::F::Pager | Rm::F::Anonymous))
    return L4Re::Env::env()->rm()->page_in(start, end - start + 1, rights);

  if (!_mem)
    return -L4_ENOENT;

  L4Re::Rm::Region_flags rm_flags = (flags() & ~L4Re::Rm::F::Rights_mask)
                                  | rights;
  L4Re::Dataspace::Flags ds_flags = map_flags(rm_flags);

  if (_flags & Rm::F::Private)
    {
#ifdef CONFIG_MMU
      for (start = l4_trunc_page(start);
           start <= end && end - start >= L4_PAGESIZE - 1;
           start += L4_PAGESIZE)
        if (l4_ret_t err = map_cow_page(r, ds_flags, start); err < 0)
          return err;

      return L4_EOK;
#else
      l4_addr_t anon_offset = start - r.start() + _anon_offs;
      return _anon_mem->map_region(anon_offset, ds_flags, start, end + 1);
#endif
    }
  else
    {
      L4::Cap<L4Re::Dataspace> ds = _mem.itas_cap();
      l4_addr_t offset = start - r.start() + _offs;
      return ds->map_region(offset, ds_flags, start, end + 1);
    }
}

void
Region_handler::free(l4_addr_t start, unsigned long size) const noexcept
{
#ifdef CONFIG_MMU
  // Anonymous mappings are completely handled by the moe region manager.
  if (_flags & Rm::F::Anonymous)
    return;

  // A part of a private mapping was detached. Access to the private pages
  // has been lost. Evict them to free the memory.
  assert(_flags & Rm::F::Private);
#else
  // Anonymous and/or private mapping was detached...
  assert(_flags & (Rm::F::Anonymous | Rm::F::Private));
#endif

  _anon_mem->clear(_anon_offs + start, size);
}

l4_ret_t
Region_handler::map_info(l4_addr_t *start_addr, l4_addr_t *end_addr) const noexcept
{
  if (_flags & (Rm::F::Pager | Rm::F::Reserved | Rm::F::Kernel))
    return 0;

  l4_ret_t ret = 0;
  if (_anon_mem)
    {
      ret = _anon_mem->map_info(start_addr, end_addr);
      if (ret > 0)
        *start_addr += _anon_offs;
    }
  else if (_mem)
    {
      ret = _mem.itas_cap()->map_info(start_addr, end_addr);
      if (ret > 0)
        *start_addr += _offs;
    }

  return ret;
}

bool
Region_handler::attached(l4_addr_t beg, l4_addr_t end) noexcept
{
  // Stack regions must be attached with the moe region manager. The page fault
  // trampoline is executing on the stack and thus must be pagable via
  // page fault IPC!
  //
  // Unfortunately, we don't have the information here if the dataspace is used
  // as stack memory. To be on the safe side, attach all shared mapping regions
  // that are backed by a real dataspace. Anonymous mappings (regardless if
  // shared or private) are always provided by the moe region manager and
  // attached there.

  // Reserved or kernel regions are not paged at all.
  if (_flags & (Rm::F::Kernel | Rm::F::Reserved))
    return true;

  // Is this region is just a sentinel for a replicated region of the moe
  // region manger when we started? See insert_regions() in main.cpp.
  if (_flags & Rm::F::Pager)
     return true;

  if (_attached)
    return true;

  auto moe_rm = L4Re::Env::env()->rm();
  l4_ret_t res;

  if (_anon_mem)
    {
#ifdef CONFIG_MMU
      // If ITAS is doing CoW (private mapping of a dataspace), the moe region
      // manager is not involved at all.
      return true;
#else
      // ITAS has allocated an anonymous dataspace. Attach the relevant part
      // but hide Anonymous and Private flags because moe would replicate our
      // work and fail.
      res = moe_rm->attach(&beg, end - beg + 1,
                           _flags & ~(Rm::F::Anonymous | Rm::F::Private),
                           L4::Ipc::make_cap_rw(_anon_mem.get()), _anon_offs);
#endif
    }
  else
    res = moe_rm->attach(&beg, end - beg + 1, _flags,
                         L4::Ipc::make_cap_rw(_mem.itas_cap()), _offs);

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

bool
Region_handler::detached(l4_addr_t beg, l4_addr_t end) const noexcept
{
  if (_attached)
    {
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

  unmap_page_range(beg, end - beg + 1);
  return true;
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
      char r[9];
      r[0] = f & L4Re::Rm::F::Anonymous   ? 'A' : '-';
      r[1] = f & L4Re::Rm::F::Private     ? 'P' : '-';
      r[2] = f & L4Re::Rm::F::Reserved    ? 'R' : '-';
      r[3] = f & L4Re::Rm::F::Pager       ? '^' : '-';
      r[4] = f & L4Re::Rm::F::Kernel      ? 'K' : '-';
      r[5] = f & L4Re::Rm::F::R ? 'r' : '-';
      r[6] = f & L4Re::Rm::F::W ? 'w' : '-';
      r[7] = f & L4Re::Rm::F::X ? 'x' : '-';
      r[8] = '\0';
      printf("  %010lx-%010lx ds=%04lx@%07llx %s/%04x %07llx: %.*s \n",
             i->first.start(), i->first.end(),
             i->second.memory().cap() >> L4_CAP_SHIFT, i->second.offset(),
             r, i->second.flags(),
             i->first.backing_offset(), i->first.name_len(), i->first.name());
    }
}

l4_ret_t
Region_map::page_in(l4_addr_t min_addr, l4_addr_t max_addr,
                    L4Re::Rm::Region_flags rights)
{
  // Start and end address are inclusive. They must be page aligned.
  if (   l4_trunc_page(min_addr)     != min_addr
      || l4_trunc_page(max_addr + 1) != max_addr + 1)
    return -L4_EINVAL;

  l4_addr_t addr = min_addr;
  while (addr < max_addr)
    {
      auto r = find(Region(addr));
      if (!r)
        return -L4_ENOMEM;

      // Cannot request more rights than with what the region was attached in
      // the first place.
      if ((r->second.flags() & rights) != rights)
        return -L4_EACCESS;

      l4_addr_t start = min_addr;
      l4_addr_t end = cxx::min(max_addr, r->first.end());
      l4_ret_t err = r->second.page_in(r->first, start, end, rights, nullptr);
      if (err < 0)
        return err;

      addr = r->first.end() + 1;
    }

  return L4_EOK;
}

cxx::Result<Region_map::Ds_alloc>
Region_map::alloc_ds(l4_size_t size)
{
  auto mem = L4Re::make_shared_cap<L4Re::Dataspace>(Global::cap_alloc);
  if (!mem)
    return cxx::Error(-L4_ENOMEM);

#ifdef CONFIG_MMU
  auto page_bm = Page_bitmap::create(l4_round_page(size) >> L4_PAGESHIFT);
  if (!page_bm)
    return cxx::Error(-L4_ENOMEM);
#else
  Page_bitmap_ptr page_bm{};  // not needed without MMU
#endif

  if (int err = Global::allocator->alloc(size, mem.get()); err < 0)
    return cxx::Error(err);

  return Ds_alloc{cxx::move(mem), cxx::move(page_bm)};
}

cxx::Result<Region_map::Anon_alloc>
Region_map::alloc_anon_mem(l4_size_t size)
{
  if (size >= Ds_threshold)
    {
      cxx::Result<Ds_alloc> alloc = alloc_ds(size);
      if (!alloc)
        return cxx::Error(alloc.error());

      return Anon_alloc{cxx::move(alloc.result().ds),
                        cxx::move(alloc.result().page_bm),
                        0};
    }

  size = l4_round_page(size);
  if (!_anon_mem || size > Ds_pool_size - _anon_offset)
    {
      cxx::Result<Ds_alloc> alloc = alloc_ds(Ds_pool_size);
      if (!alloc)
        return cxx::Error(alloc.error());

      _anon_mem = cxx::move(alloc.result().ds);
      _anon_page_bm = cxx::move(alloc.result().page_bm);
      _anon_offset = 0;
    }

  cxx::Result<Anon_alloc> res(Anon_alloc{_anon_mem, _anon_page_bm, _anon_offset});
  _anon_offset += size;
  return res;
}


void Region_map_svr::init_cow()
{
#ifdef CONFIG_MMU
  cow_area = _region_map.attach_area(0, L4_PAGESIZE,
                                     L4Re::Rm::F::Reserved
                                     | L4Re::Rm::F::Search_addr);
  if (cow_area == L4_INVALID_ADDR)
    L4Re::throw_error(-L4_ENOMEM, "Cannot reserve CoW area");
#endif
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

l4_ret_t
Region_map_svr::page_in(l4_addr_t min_addr, l4_addr_t max_addr,
                        L4Re::Rm::Region_flags rights)
{
  Rw_lock_read_scope scope(_lock);
  return _region_map.page_in(min_addr, max_addr, rights);
}

FFI_DEFINE_CLASS_FN(l4_ret_t, Region_map_svr, thread_page_in_handler,
                    l4_addr_t min_addr, l4_addr_t max_addr,
                    L4Re::Rm::Region_flags rights)
{
  auto self = Global::local_rm.get();
  return self->page_in(min_addr, max_addr, rights);
}
