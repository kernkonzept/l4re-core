/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/types.h>

#include <l4/re/util/region_mapping>
#include <l4/re/util/region_mapping_svr_2>
#include <l4/sys/cxx/ipc_epiface>

#include "dataspace.h"
#include "quota.h"
#include "debug.h"

class Region_ops;


typedef L4Re::Util::Region_handler<cxx::Weak_ref<Moe::Dataspace const>,
                                   Region_ops> Region_handler;

class Region_ops
{
public:
  typedef L4::Ipc::Snd_fpage Map_result;
  static int map(Region_handler const *h, l4_addr_t adr,
                 L4Re::Util::Region const &r, bool writable,
                 L4::Ipc::Snd_fpage *result);
  static void unmap(Region_handler const * /*h*/, l4_addr_t /*vaddr*/,
                    l4_addr_t /*offs*/, unsigned long /*size*/)
  {}

  static void free(Region_handler const *h, l4_addr_t start, unsigned long size);

  static void take(Region_handler const * /*h*/) {}
  static void release(Region_handler const * /*h*/) {}

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
  static int validate_ds(void *, L4::Ipc::Snd_fpage const &ds_cap,
                         unsigned flags, Dataspace *ds);
  static l4_umword_t find_res(Dataspace const &) { return 0; }

  Region_map();
  virtual ~Region_map() {}

  long op_io_page_fault(L4::Io_pager::Rights,
                        l4_fpage_t io_pfa, l4_umword_t pc,
                        L4::Ipc::Opt<l4_mword_t> &result,
                        L4::Ipc::Opt<L4::Ipc::Snd_fpage> &);
};

