/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test moe's usage of the sigma0 capability.
 */

#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/atkins/debug>
#include <l4/atkins/factory>

#include <l4/sys/kip>
#include <l4/re/env>
#include <l4/sys/types.h>
#include <l4/sys/consts.h>
#include <l4/sigma0/sigma0.h>

using namespace L4::Kip;

/**
 * Fixture with sigma0 cap and simple kip memory descriptor search.
 */
class Sigma0Client : public testing::Test
{
public:
  Sigma0Client() : vaddr(0) {}

  void SetUp() override
  {
    L4Re::chksys(L4Re::Env::env()->rm()->reserve_area(
                   &vaddr, 0x10000000,
                   L4Re::Rm::F::Reserved | L4Re::Rm::F::Search_addr),
                 "Reserve a virtual address space for test mappings.");

    sigma0_cap = L4Re::chkcap(L4Re::Env::env()->get_cap<void>("sigma0"),
                              "Get sigma0 capability.");
  }

  void TearDown() override
  {
    L4Re::chksys(L4Re::Env::env()->rm()->free_area(vaddr),
                 "Free virtual address space used for test mappings.");
  }

protected:
  L4::Cap<void> sigma0_cap;
  l4_addr_t vaddr;

  static Atkins::Dbg info() { return Atkins::Dbg(Atkins::Dbg::Info, "sigma0"); }

  /**
   * Simplistic search function to find a block of RAM
   *
   * \param sz        Size of the memory region to find.
   * \param[out] mem  Returns the address of the memory region found or 0 if
   *                  not found.
   * \return  0 for success, <0 otherwise.
   *
   * \note  The first RAM address found is naively selected. No check is made
   *        to see if the requested RAM block is entirely within the RAM region
   *        or overlapping with a reserved region. Unless the KIP specifies a
   *        reserved block at the top of RAM this should be adequate for the
   *        purposes of these tests.
   */
  int get_ram(l4_addr_t sz, l4_addr_t *mem)
  {
    Mem_desc::Mem_type const type = Mem_desc::Conventional;
    int ret = -L4_ENOMEM;

    *mem = 0;
    for (auto const &md : Mem_desc::all(l4re_kip()))
      {
        if (!md.type() || md.type() != type || md.is_virtual())
          continue;

        info().printf("mem_desc: %stype %2d start %lx end %lx sz %lx\n",
                      md.is_virtual() ? "v " : "", md.type(), md.start(),
                      md.end(), md.size());

        if ((md.end() - md.start()) >= sz)
          {
            *mem = md.start();
            ret = L4_EOK;
            break;
          }
      }

    info().printf("return: %d type %2d mem %lx sz %lx\n", ret, type, *mem, sz);

    return ret;
  }
};

/**
 * The debug dump API can be called by sigma0 client.
 *
 * Also helps to see what memory regions are available in the KIP.
 */
TEST_F(Sigma0Client, DebugDump)
{
  l4sigma0_debug_dump(sigma0_cap.cap());

  for (auto const &md : Mem_desc::all(l4re_kip()))
    {
      info().printf("%stype %2d start %#lx end %#lx\n",
                    md.is_virtual() ? "v " : "", md.type(), md.start(),
                    md.end());
    }
}

/**
 * A previously allocated free RAM page may not be remapped using sigma0 client
 * made available in root namespace.
 *
 * Request a free page using the l4sigma0_map_anypage API and try to map it.
 *
 * \note All free pages are allocated by Moe and are not expected to be
 *       available. This would still be the case if Moe were to give out its
 *       own sigma0 cap instead of creating a new one for the initial task.
 */
TEST_F(Sigma0Client, RemapAnyPageMappingAlreadyInMoe)
{
  l4_addr_t phys_base = 0;

  ASSERT_EQ(-L4SIGMA0_NOFPAGE,
            l4sigma0_map_anypage(sigma0_cap.cap(), vaddr, L4_LOG2_PAGESIZE,
                                 &phys_base, L4_LOG2_PAGESIZE))
    << "Map any free RAM page which is already allocated to Moe.";
}

/**
 * RAM memory previously allocated by Moe may not be remapped using sigma0
 * client made available in root namespace.
 *
 * \note This test would fail (and the mapping would succeed) if Moe were to
 *       give out its own sigma0 cap instead of creating a new one for the
 *       initial task.
 */
TEST_F(Sigma0Client, MapMemoryAlreadyMappedByMoe)
{
  l4_addr_t mem_phys;
  l4_addr_t const mem_size = L4_PAGESIZE;

  ASSERT_L4OK(get_ram(mem_size, &mem_phys));

  ASSERT_EQ(-L4SIGMA0_NOFPAGE,
            l4sigma0_map_mem(sigma0_cap.cap(), mem_phys, vaddr, mem_size))
    << "Map RAM memory which is already allocated to Moe.";
}
