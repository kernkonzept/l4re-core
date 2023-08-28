/*
 * Copyright (C) 2025 Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/remote_access>
#include <l4/sys/cxx/consts>

#include <limits.h>

class Remote_access : public L4::Epiface_t<Remote_access, L4Re::Remote_access>
{
public:
  l4_ret_t op_read_mem(L4Re::Remote_access::Rights,
                       l4_addr_t addr, char width, l4_uint64_t &val);
  l4_ret_t op_write_mem(L4Re::Remote_access::Rights,
                        l4_addr_t addr, char width, l4_uint64_t val);

  l4_ret_t op_terminate(L4Re::Remote_access::Rights, int exit_code);

  l4_ret_t op_map(L4Re::Dataspace::Rights r,
                  L4Re::Dataspace::Offset offset,
                  L4Re::Dataspace::Map_addr spot,
                  L4Re::Dataspace::Flags flags,
                  L4::Ipc::Snd_fpage &fp);

  l4_ret_t op_map_info(L4Re::Dataspace::Rights,
                       l4_addr_t &start_addr, l4_addr_t &end_addr)
  {
    start_addr = 0;
    end_addr = ~static_cast<l4_addr_t>(0);
    return 0;
  }

  l4_ret_t op_info(L4Re::Dataspace::Rights, L4Re::Dataspace::Stats &stats)
  {
    stats.size = L4::trunc_page(ULONG_MAX);
    stats.flags = L4Re::Dataspace::F::RW;
    return 0;
  }

  l4_ret_t op_clear(L4Re::Dataspace::Rights,
                    L4Re::Dataspace::Offset,
                    L4Re::Dataspace::Size)
  { return -L4_ENOSYS; }

  l4_ret_t op_copy_in(L4Re::Dataspace::Rights,
                      L4Re::Dataspace::Offset,
                      L4::Ipc::Snd_fpage,
                      L4Re::Dataspace::Offset,
                      L4Re::Dataspace::Size)
  { return -L4_ENOSYS; }

  l4_ret_t op_allocate(L4Re::Dataspace::Rights,
                       L4Re::Dataspace::Offset,
                       L4Re::Dataspace::Size)
  { return 0; }
};

extern Remote_access ra_if;
