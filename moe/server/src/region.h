/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>

#include <l4/re/util/region_mapping>
#include <l4/re/util/region_mapping_svr>
#include <l4/sys/cxx/ipc_epiface>

#include "dataspace.h"
#include "quota.h"
#include "debug.h"

class Region_handler;

class Region_handler
{
  L4Re::Rm::Offset _offs = 0;
  cxx::Weak_ref<Moe::Dataspace const> _mem;
  l4_cap_idx_t _client_cap = L4_INVALID_CAP;
  L4Re::Rm::Region_flags _flags;

  constexpr bool is_ro() const noexcept
  {
    return !(_flags & L4Re::Rm::F::W);
  }

  L4Re::Rm::Region_flags caching() const noexcept
  {
    return _flags & L4Re::Rm::F::Caching_mask;
  }

public:
  using Map_result = L4::Ipc::Snd_fpage;
  using Dataspace = cxx::Weak_ref<Moe::Dataspace const>;

  Region_handler() noexcept : _flags() {}
  Region_handler(cxx::Weak_ref<Moe::Dataspace const> const &mem, l4_cap_idx_t client_cap,
      L4Re::Rm::Offset offset = 0,
      L4Re::Rm::Region_flags flags = L4Re::Rm::Region_flags(0)) noexcept
    : _offs(offset), _mem(mem), _client_cap(client_cap), _flags(flags)
  {}

  cxx::Weak_ref<Moe::Dataspace const> const &memory() const noexcept
  {
    return _mem;
  }

  l4_cap_idx_t client_cap_idx() const noexcept
  {
    return _client_cap;
  }

  L4Re::Rm::Offset offset() const noexcept
  {
    return _offs;
  }

  L4Re::Rm::Region_flags flags() const noexcept
  {
    return _flags;
  }

  Region_handler operator + (l4_int64_t offset) const noexcept
  {
    Region_handler n = *this; n._offs += offset; return n;
  }

  void free(l4_addr_t start, unsigned long size) const noexcept;

  l4_ret_t map(l4_addr_t addr, L4Re::Util::Region const &r, bool writable,
               Map_result *result) const noexcept;

  l4_ret_t map_info(l4_addr_t *start_addr, l4_addr_t *end_addr) const noexcept;

  bool attached(l4_addr_t, l4_addr_t) const noexcept
  { return true; }
  bool detached(l4_addr_t, l4_addr_t) const noexcept
  { return false; }
};

class Region_map
: public L4Re::Util::Region_map<Region_handler, Moe::Quota_allocator>,
  public L4::Epiface_t<Region_map, L4Re::Rm, Moe::Server_object>,
  public L4Re::Util::Rm_server<Region_map, Dbg>,
  public Moe::Q_object
{
private:
  typedef L4Re::Util::Region_map<Region_handler, Moe::Quota_allocator> Base;

public:
  typedef cxx::Weak_ref<Moe::Dataspace const> Dataspace;
  enum { Have_find = false };
  static l4_ret_t validate_ds(L4::Ipc::Snd_fpage const &ds_cap,
                              L4Re::Rm::Region_flags flags, Dataspace *ds);
  static l4_umword_t find_res(Dataspace const &) { return 0; }

  Region_map();
  virtual ~Region_map() {}

  l4_ret_t op_io_page_fault(L4::Io_pager::Rights,
                            l4_fpage_t io_pfa, l4_umword_t pc,
                            L4::Ipc::Opt<L4::Ipc::Snd_fpage> &);
};

