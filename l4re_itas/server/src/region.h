/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/exception>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/dataspace>
#include <l4/re/util/region_mapping_svr>
#include <l4/re/debug>
#include "debug.h"
#include <stdlib.h>

inline void *operator new (size_t s, cxx::Nothrow const &) noexcept { return malloc(s); }
inline void operator delete (void *p, cxx::Nothrow const &) noexcept { return free(p); }

class Region_ops;

typedef L4Re::Util::Region_handler<L4::Cap<L4Re::Dataspace>, Region_ops> Region_handler;

class Region_ops
{
public:
  typedef l4_umword_t Map_result;
  static int map(Region_handler const *h, l4_addr_t addr,
                 L4Re::Util::Region const &r, bool writable,
                 l4_umword_t *result);

  static void free(Region_handler const *h, l4_addr_t start, unsigned long size);
  static int map_info(Region_handler const *h,
                      l4_addr_t *start_addr, l4_addr_t *end_addr);
};


using Region_map_interface =
  L4::Kobject_3t<void, L4Re::Rm, L4::Exception, L4Re::Debug_obj>;


class Region_map
: public L4Re::Util::Region_map<Region_handler, cxx::New_allocator>,
  public L4::Epiface_t<Region_map, Region_map_interface>,
  public L4Re::Util::Rm_server<Region_map, Dbg>
{
private:
  typedef L4Re::Util::Region_map<Region_handler, cxx::New_allocator> Base;

public:
  typedef L4::Cap<L4Re::Dataspace> Dataspace;
  enum { Have_find = true };
  static int validate_ds(void *, L4::Ipc::Snd_fpage const &ds_cap,
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

  int op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                   L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp);
  long op_io_page_fault(L4::Io_pager::Rights,
                        l4_fpage_t io_pfa, l4_umword_t pc,
                        L4::Ipc::Opt<L4::Ipc::Snd_fpage> &);
  long op_debug(L4Re::Debug_obj::Rights, unsigned long function)
  { debug_dump(function); return 0; }
};


