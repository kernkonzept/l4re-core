/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <stdlib.h>

#include <l4/re/dataspace>
#include <l4/re/debug>
#include <l4/re/util/region_mapping_svr>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/exception>
#include <l4/sys/types.h>
#include <l4/sys/pf_trampoline.h>
#include <l4/util/thread.h>

#include "debug.h"
#include "lock.h"

inline void *operator new (size_t s, cxx::Nothrow const &) noexcept { return malloc(s); }
inline void operator delete (void *p, cxx::Nothrow const &) noexcept { return free(p); }

class Region_handler
{
  L4Re::Rm::Offset _offs = 0;
  L4::Cap<L4Re::Dataspace> _mem;
  l4_cap_idx_t _client_cap = L4_INVALID_CAP;
  L4Re::Rm::Region_flags _flags = L4Re::Rm::Region_flags(0);
  bool _attached = false;

public:
  typedef l4_umword_t Map_result;
  using Dataspace = L4::Cap<L4Re::Dataspace>;

  Region_handler() noexcept = default;
  Region_handler(L4::Cap<L4Re::Dataspace> mem,
                 l4_cap_idx_t client_cap,
                 L4Re::Rm::Offset offset,
                 L4Re::Rm::Region_flags flags) noexcept
  : _offs(offset), _mem(mem), _client_cap(client_cap), _flags(flags)
  {}

  L4::Cap<L4Re::Dataspace> memory() const noexcept
  { return _mem; }

  l4_cap_idx_t client_cap_idx() const noexcept
  { return _client_cap; }

  L4Re::Rm::Offset offset() const noexcept
  { return _offs; }

  L4Re::Rm::Region_flags flags() const noexcept
  { return _flags; }

  Region_handler operator + (l4_int64_t offset) const noexcept
  {
    Region_handler n = *this; n._offs += offset; return n;
  }

  void free(l4_addr_t start, unsigned long size) const noexcept;

  l4_ret_t map(l4_addr_t addr, L4Re::Util::Region const &r, bool writable,
               Map_result *result) const noexcept;

  l4_ret_t map_info(l4_addr_t *start_addr, l4_addr_t *end_addr) const noexcept;

  bool attached(l4_addr_t beg, l4_addr_t end) noexcept;
  bool detached(l4_addr_t beg, l4_addr_t end) const noexcept;
};


using Region_map_interface =
  L4::Kobject_3t<void, L4Re::Rm, L4::Exception, L4Re::Debug_obj>;


class Region_map
: public L4Re::Util::Region_map<Region_handler, cxx::New_allocator>,
  public L4Re::Util::Rm_server<Region_map, Dbg>
{
private:
  typedef L4Re::Util::Region_map<Region_handler, cxx::New_allocator> Base;

public:
  typedef L4::Cap<L4Re::Dataspace> Dataspace;
  enum { Have_find = true };
  static l4_ret_t validate_ds(L4::Ipc::Snd_fpage const &ds_cap,
                              L4Re::Rm::Region_flags,
                              L4::Cap<L4Re::Dataspace> *ds)
  {
    // if no cap was sent then the region will be reserved
    if (ds_cap.local_id_received())
      {
	// we received a local capability index, get it with cap.base()
	*ds = L4::Cap<L4Re::Dataspace>(ds_cap.base());
	return L4_EOK;
      }
    return -L4_ENOENT;
  }

  static l4_umword_t find_res(L4::Cap<void> const &ds) { return ds.cap(); }

  Region_map();
  virtual ~Region_map() {}

  void init();

  void debug_dump(unsigned long function) const;
};

class Region_map_svr
: public L4::Epiface_t<Region_map_svr, Region_map_interface>
{
public:
  class Read_access
  {
    Region_map const &_rm;
    Rw_lock_read_scope _read_scope;

  public:
    explicit Read_access(Region_map const &rm, Rw_lock &lock)
    : _rm(rm), _read_scope(lock)
    {}

    Region_map const *operator->() const { return &_rm; }
  };

  void init() { _region_map.init(); }

  bool add_area(l4_addr_t start, l4_addr_t end,
                L4Re::Rm::F::Region_flags region_flags)
  { return _region_map.add_area(L4Re::Util::Region(start, end), region_flags); }

  bool add_region(l4_addr_t start, l4_addr_t end, L4::Cap<L4Re::Dataspace> mem,
                  L4Re::Rm::F::Region_flags region_flags, char const *name,
                  unsigned name_len, L4Re::Rm::Offset backing_offset)
  {
    L4Re::Util::Region region(start, end, name, name_len, backing_offset);
    Region_handler handler(mem, mem.cap(), 0, region_flags);
    return _region_map.add_region(region, handler);
  }

  l4_addr_t attach_area(l4_addr_t addr, unsigned long size,
                   L4Re::Rm::Flags flags = L4Re::Rm::Flags(0)) noexcept
  {
    Rw_lock_write_scope scope(_lock);
    return _region_map.attach_area(addr, size, flags);
  }

  bool detach_area(l4_addr_t addr) noexcept
  {
    Rw_lock_write_scope scope(_lock);
    return _region_map.detach_area(addr);
  }

  void *attach(void *addr, l4_size_t size, L4::Cap<L4Re::Dataspace> mem,
               L4Re::Rm::Flags flags, char align = L4_PAGESHIFT,
               char const *name = nullptr, unsigned name_len = 0,
               L4Re::Rm::Offset backing_offset = 0)
  {
    Rw_lock_write_scope scope(_lock);

    return _region_map.attach(addr, size,
                              Region_handler(mem, mem.cap(), 0,
                                             flags.region_flags()),
                              flags.attach_flags(), align,
                              name, name_len, backing_offset);
  }

  Read_access read_access() const
  { return Read_access(_region_map, _lock); }

  L4UTIL_THREAD_CXX_FUNC_PROTO(thread_pf_handler,
                               l4_pf_trampoline_t *tramp, l4_addr_t addr);

  // L4Re::Rm API

  long op_attach(L4Re::Rm::Rights rights, l4_addr_t &start,
                 unsigned long size, L4Re::Rm::Flags flags,
                 L4::Ipc::Snd_fpage ds_cap, L4Re::Rm::Offset offs,
                 unsigned char align, l4_cap_idx_t client_cap_idx,
                 L4::Ipc::String<> name, L4Re::Rm::Offset backing_offset)
  {
    Rw_lock_write_scope scope(_lock);
    return _region_map.op_attach(rights, start, size, flags, ds_cap, offs,
                                 align, client_cap_idx, name, backing_offset);
  }

  long op_free_area(L4Re::Rm::Rights rights, l4_addr_t start)
  {
    Rw_lock_write_scope scope(_lock);
    return _region_map.op_free_area(rights, start);
  }

  long op_find(L4Re::Rm::Rights rights, l4_addr_t &addr, unsigned long &size,
               L4Re::Rm::Flags &flags, L4Re::Rm::Offset &offset,
               L4::Cap<L4Re::Dataspace> &m)
  {
    Rw_lock_read_scope scope(_lock);
    return _region_map.op_find(rights, addr, size, flags, offset, m);
  }

  long op_detach(L4Re::Rm::Rights rights, l4_addr_t addr,
                 unsigned long size, unsigned flags,
                 l4_addr_t &start, l4_addr_t &rsize,
                 l4_cap_idx_t &mem_cap)
  {
    Rw_lock_write_scope scope(_lock);
    return _region_map.op_detach(rights, addr, size, flags, start, rsize,
                                 mem_cap);
  }

  long op_reserve_area(L4Re::Rm::Rights rights, l4_addr_t &start,
                       unsigned long size, L4Re::Rm::Flags flags,
                       unsigned char align)
  {
    Rw_lock_write_scope scope(_lock);
    return _region_map.op_reserve_area(rights, start, size, flags, align);
  }

  long op_get_regions(L4Re::Rm::Rights rights, l4_addr_t addr,
                      L4::Ipc::Ret_array<L4Re::Rm::Region> regions)
  {
    Rw_lock_read_scope scope(_lock);
    return _region_map.op_get_regions(rights, addr, regions);
  }

  long op_get_areas(L4Re::Rm::Rights rights, l4_addr_t addr,
                    L4::Ipc::Ret_array<L4Re::Rm::Area> areas)
  {
    Rw_lock_read_scope scope(_lock);
    return _region_map.op_get_areas(rights, addr, areas);
  }

  long op_get_info(L4Re::Rm::Rights rights, l4_addr_t addr,
                   L4::Ipc::String<char> &name,
                   L4Re::Rm::Offset &backing_offset)
  {
    Rw_lock_read_scope scope(_lock);
    return _region_map.op_get_info(rights, addr, name, backing_offset);
  }

  // L4::Pager API

  l4_ret_t op_page_fault(L4::Pager::Rights rights, l4_umword_t addr,
                         l4_umword_t pc, L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp)
  {
    Rw_lock_read_scope scope(_lock);
    return _region_map.op_page_fault(rights, addr, pc, fp);
  }

  l4_ret_t op_io_page_fault(L4::Io_pager::Rights,
                            l4_fpage_t io_pfa, l4_umword_t pc,
                            L4::Ipc::Opt<L4::Ipc::Snd_fpage> &);

  // L4::Exception API

  l4_ret_t op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                        L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp);

  // L4Re::Debug_obj API

  l4_ret_t op_debug(L4Re::Debug_obj::Rights, unsigned long function)
  {
    Rw_lock_read_scope scope(_lock);
    _region_map.debug_dump(function);
    return 0;
  }

private:
  Region_map _region_map;
  mutable Rw_lock _lock;
};


