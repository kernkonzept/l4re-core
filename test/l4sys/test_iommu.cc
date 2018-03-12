/*
 * Copyright (C) 2016  Kernkonzept GmbH.
 * Author: Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include <l4/re/dma_space>
#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/sys/debugger.h>
#include <l4/sys/err.h>
#include <l4/sys/factory>
#include <l4/sys/iommu>
#include <l4/sys/kdebug.h>
#include <l4/util/util.h>

#include <l4/atkins/tap/main>

#include <cstdio>
#include <stdlib.h>

char mem[4096] __attribute__((aligned(4096)));

TEST(IOMMU, Mappings)
{
  int ret;
  enum {
      NR        = 1024,
      PAGE_SIZE = 4096,
  };

  printf("Hello from iommu testcase.\n");

  auto iommu     = L4Re::Env::env()->get_cap<L4::Iommu>("iommu");
  auto dma_space = L4Re::Util::make_auto_del_cap<L4::Task>();

  if (!iommu.is_valid())
    {
      RecordProperty("SKIP", "this system does not have an iommu");
      return;
    }

  ASSERT_TRUE(dma_space.get().is_valid())
    << "dma_space cap invalid";

  ret = l4_error(l4_factory_create_u(L4Re::Env::env()->factory().cap(),
                             L4_PROTO_DMA_SPACE,
                             dma_space.get().cap(),
                             l4_utcb()));
  ASSERT_EQ(0, ret) << "create dma space failed: "
    << ret << " " << l4sys_errtostr(ret);

  ret = l4_error(iommu->bind((2 << 18) | (15 << 8) | 1, dma_space.get()));
  ASSERT_EQ(0, ret) << "bind iommu failed: "
    << ret << " " << l4sys_errtostr(ret);

  l4_touch_ro(mem, 4096);

  printf("mem @ %p\n", (void*)mem);

  for (unsigned i = 0; i < NR; ++i)
    {
      ret = l4_error(dma_space.get()->map(L4Re::Env::env()->task(),
                                l4_fpage((unsigned long)mem, 12, L4_FPAGE_RWX),
                                i * PAGE_SIZE));
      ASSERT_EQ(0, ret) << "map page failed: "
        << ret << " " << l4sys_errtostr(ret);
    }

  ret = l4_error(L4Re::Env::env()->task()->unmap(l4_fpage((unsigned long)mem, 12, L4_FPAGE_RWX),
                                         L4_FP_OTHER_SPACES));
  ASSERT_EQ(0, ret) << "unmap page failed: "
    << ret << " " << l4sys_errtostr(ret);

  printf("Done. Bye.\n");
}

