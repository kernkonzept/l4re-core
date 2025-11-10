/*
 * Copyright (C) 2008-2009, 2025 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *            Alexander Warg <warg@os.inf.tu-dresden.de>
 *            Martin Decky <martin.decky@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/bid_config.h>
#include <l4/sys/consts.h>
#include <l4/re/dataspace>
#include <l4/re/mem_alloc>
#include <l4/re/error_helper>
#include <l4/re/unique_cap>
#include <l4/cxx/iostream>
#include <l4/libumalloc/umalloc.h>
#include "globals.h"

extern char __executable_start[];

size_t umalloc_area_granularity = 64 * L4_PAGESIZE;

#if defined(CONFIG_MMU)
static l4_addr_t umalloc_pos = reinterpret_cast<l4_addr_t>(__executable_start)
                               + 0x100000;
#endif

static void *err_msg(char const *msg, size_t area_size, long err = 0)
{
  printf("l4re_itas: ERROR: umalloc_area_create(%zd): %s", area_size, msg);

  if (err)
    printf(": %s (%ld)", l4sys_errtostr(err), err);;

  printf("\n");
  return nullptr;
}

void *umalloc_area_create(size_t area_size) noexcept
{
  // Allocate a dataspace.
  auto ds = L4Re::make_unique_cap<L4Re::Dataspace>(Global::cap_alloc);
  if (!ds.is_valid())
    return err_msg("Failed to allocate dataspace capability", area_size);

  // Allocate physical memory.
  auto err = Global::allocator->alloc(area_size, ds.get());
  if (err < 0)
    return err_msg("Failed to allocate memory", area_size, err);

  L4Re::Rm::Flags flags(L4Re::Rm::F::RW);

#if defined(CONFIG_MMU)
  l4_addr_t start = umalloc_pos;
#else
  l4_addr_t start = 0;
  flags |= L4Re::Rm::F::Search_addr;
#endif

  // Attach the memory.
  err = L4Re::Env::env()->rm()->attach(&start, area_size, flags,
                                       L4::Ipc::make_cap_rw(ds.get()),
                                       0, L4_PAGESHIFT,
                                       L4::Cap<L4::Task>::Invalid,
                                       "[itas-heap]");
  if (err < 0)
    return err_msg("Failed to attach memory", area_size, err);

#if defined(CONFIG_MMU)
  // Note that the starting address is guaranteed to be page-aligned.
  umalloc_pos = start + l4_round_page(area_size);
#endif

  // The current implementation of libumalloc does not dispose of unused/empty
  // heap areas. When this is implemented eventually, we will have to retain
  // the dataspace capability for the possible future disposal.
  //
  // So far, we just explicitly leak the dataspace capability here. The
  // capability will be automatically disposed during program termination.
  ds.release();

  return reinterpret_cast<void *>(start);
}
