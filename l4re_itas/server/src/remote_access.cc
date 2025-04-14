/*
 * Copyright (C) 2025 Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <cstdio>
#include <unistd.h>

#include "globals.h"
#include "region.h"
#include "remote_access.h"

Remote_access ra_if;

long Remote_access::op_read_mem(L4Re::Remote_access::Rights,
                                l4_addr_t addr, char width, l4_uint64_t &val)
{
  // This is an intermediate solution, to be improved.
  L4::Ipc::Opt<L4::Ipc::Snd_fpage> fp;
  long ret = Global::local_rm->op_page_fault(0, addr & ~7ul, 0, fp);
  if (ret)
    return ret;

  switch (width)
    {
    case L4Re::Remote_access::Wd_8bit:
      val = *reinterpret_cast<l4_uint8_t  *>(addr); break;
    case L4Re::Remote_access::Wd_16bit:
      val = *reinterpret_cast<l4_uint16_t *>(addr); break;
    case L4Re::Remote_access::Wd_32bit:
      val = *reinterpret_cast<l4_uint32_t *>(addr); break;
    case L4Re::Remote_access::Wd_64bit:
      val = *reinterpret_cast<l4_uint64_t *>(addr); break;
    default:
      return -L4_EINVAL;
    };

  return 0;
}

long Remote_access::op_write_mem(L4Re::Remote_access::Rights,
                                 l4_addr_t addr, char width, l4_uint64_t val)
{
  printf("Remote_access::op_write_mem(%lx, %d, %llx): Not yet.\n", addr, width, val);
  return -L4_ENOSYS;
}

long Remote_access::op_terminate(L4Re::Remote_access::Rights, int exit_code)
{
  exit(exit_code);
  return 0;
}
