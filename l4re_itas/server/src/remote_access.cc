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
#include "safe_memcpy.h"

Remote_access ra_if;

static l4_ret_t pagein(l4_addr_t addr, bool with_write, bool with_exec)
{
  // Get read-only access to region map. Needs to be stored in variable to
  // retain read-lock until end of scope!
  auto rm = Global::local_rm->read_access();

  Region_map::Node node = rm->find(addr);
  if (!node)
    return -L4_ENOENT;

  if (node->second.flags() & (L4Re::Rm::F::Kernel | L4Re::Rm::F::Reserved))
    return 0;

  addr &= ~7ul;
  addr |= with_write ? 2 : 0;
  addr |= with_exec  ? 4 : 0;
  L4::Ipc::Opt<L4::Ipc::Snd_fpage> fp;
  // We could split op_page_fault to avoid the find in there which we just did.
  return Global::local_rm->op_page_fault(0, addr, 0, fp);
}

l4_ret_t Remote_access::op_read_mem(L4Re::Remote_access::Rights,
                                    l4_addr_t addr, char width, l4_uint64_t &val)
{
  for (bool paged_in = false;; paged_in = true)
    {
      bool smr;

      switch (width)
        {
        case L4Re::Remote_access::Wd_8bit:
          {
            l4_uint8_t v = 0;
            smr = safe_memcpy(&v, reinterpret_cast<char const *>(addr), 1);
            val = v;
            break;
          }
        case L4Re::Remote_access::Wd_16bit:
          {
            l4_uint16_t v = 0;
            smr = safe_memcpy(&v, reinterpret_cast<char const *>(addr), 2);
            val = v;
            break;
          }
        case L4Re::Remote_access::Wd_32bit:
          {
            l4_uint32_t v = 0;
            smr = safe_memcpy(&v, reinterpret_cast<char const *>(addr), 4);
            val = v;
            break;
          }
        case L4Re::Remote_access::Wd_64bit:
          smr = safe_memcpy(&val, reinterpret_cast<char const *>(addr), 8);
          break;
        default:
          return -L4_EINVAL;
        };

      if (smr)
        return 0;

      if (paged_in)
        return -L4_EFAULT;

      if (l4_ret_t r = pagein(addr, 0, 0))
        return r;
    }
}

l4_ret_t Remote_access::op_write_mem(L4Re::Remote_access::Rights,
                                     l4_addr_t addr, char width, l4_uint64_t val)
{
  printf("Remote_access::op_write_mem(%lx, %d, %llx): Not yet.\n", addr, width, val);
  return -L4_ENOSYS;
}

l4_ret_t Remote_access::op_terminate(L4Re::Remote_access::Rights, int exit_code)
{
  exit(exit_code);
  return 0;
}

l4_ret_t Remote_access::op_map(L4Re::Dataspace::Rights,
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

  l4_addr_t last_plus_1_page = offset + (1UL << order);
  for (l4_addr_t page = offset; page < last_plus_1_page; page += L4_PAGESIZE)
    if (l4_ret_t r = pagein(page, rights & L4_FPAGE_W, 0))
      return r;

  fp = L4::Ipc::Snd_fpage::mem(l4_trunc_size(offset, order), order,
                               rights, l4_trunc_page(spot),
                               L4::Ipc::Snd_fpage::Map,
                               f);
  return 0;
}
