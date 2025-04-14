/*
 * Copyright (C) 2025 Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/remote_access>

class Remote_access : public L4::Epiface_t<Remote_access, L4Re::Remote_access>
{
public:
  long op_read_mem(L4Re::Remote_access::Rights,
                   l4_addr_t addr, char width, l4_uint64_t &val);
  long op_write_mem(L4Re::Remote_access::Rights,
                    l4_addr_t addr, char width, l4_uint64_t val);

  long op_terminate(L4Re::Remote_access::Rights, int exit_code);
};

extern Remote_access ra_if;
