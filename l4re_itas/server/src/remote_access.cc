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

static l4_addr_t last_pfn = ~0ul;

// This is an intermediate solution, to be improved.
static long pagein(l4_addr_t addr, bool with_write, bool with_exec)
{
  if (l4_trunc_page(addr) != last_pfn)
    {
      L4Re::Util::Region region(addr, addr);
      Region_map::Node node = Global::local_rm->find(region);
      if (!node)
        return -L4_ENOENT;

      if (!(  node->second.flags()
            & (L4Re::Rm::F::Kernel | L4Re::Rm::F::Reserved)))
        {
          addr &= ~7ul;
          addr |= with_write ? 2 : 0;
          addr |= with_exec  ? 4 : 0;
          L4::Ipc::Opt<L4::Ipc::Snd_fpage> fp;
          // We could split op_page_fault function to avoid the find in
          // there which we just did.
          long ret = Global::local_rm->op_page_fault(0, addr, 0, fp);
          if (ret)
            return ret;
        }

      last_pfn = l4_trunc_page(addr);
    }
  return 0;
}

long Remote_access::op_read_mem(L4Re::Remote_access::Rights,
                                l4_addr_t addr, char width, l4_uint64_t &val)
{
  if (long r = pagein(addr, 0, 0))
    return r;

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

int Remote_access::op_map(L4Re::Dataspace::Rights,
                          L4Re::Dataspace::Offset offset,
                          L4Re::Dataspace::Map_addr spot,
                          [[maybe_unused]] L4Re::Dataspace::Flags flags,
                          L4::Ipc::Snd_fpage &fp)
{
  offset = l4_trunc_page(offset);

  unsigned long sz = L4_PAGESIZE;
  unsigned char order
    = l4_fpage_max_order(L4_PAGESHIFT, offset, offset, offset + sz, spot);

  L4::Ipc::Snd_fpage::Cacheopt f = L4::Ipc::Snd_fpage::Cached;

  unsigned char rights = L4_FPAGE_RO;
  // rights |= L4_FPAGE_W;

  l4_addr_t last_plus_1_page = offset + (L4_PAGESHIFT << order);
  for (l4_addr_t page = offset; page < last_plus_1_page; page += L4_PAGESIZE)
    if (long r = pagein(offset, rights & L4_FPAGE_W, 0))
      return r;

  fp = L4::Ipc::Snd_fpage::mem(l4_trunc_size(offset, order), order,
                               rights, l4_trunc_page(spot),
                               L4::Ipc::Snd_fpage::Map,
                               f);
  return 0;
}
