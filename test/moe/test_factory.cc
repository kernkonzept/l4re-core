/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Tests for the factory type.
 *
 * Only covers tests related to using a custom factory type. Full tests
 * for object creation are in the test suite for the respective object type.
 */

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/dataspace>
#include <l4/re/dma_space>
#include <l4/re/namespace>
#include <l4/sys/thread>
#include <l4/sys/vcon>
#include <l4/sys/factory>
#include <l4/sys/scheduler>
#include <l4/util/util.h>
#include <l4/re/error_helper>
#include <l4/sys/irq>
#include <l4/sys/vm>

#include <l4/atkins/tap/main>

#include "moe_helpers.h"


struct TestFactory : testing::Test {};

/**
 * Requesting a namespace yields a namespace object.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, CreateCheckNameSpace)
{
  auto f = create_fab();

  auto ns = make_unique_del_cap<L4Re::Namespace>();
  EXPECT_EQ(0, l4_error(f->create(ns.get())));
  EXPECT_EQ(-L4_ENOENT, ns->unlink("foobar"));
}

/**
 * Requesting a region manager yields a region manager object.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, CreateCheckRegionManager)
{
  auto f = create_fab();

  auto rm = make_unique_del_cap<L4Re::Rm>();
  EXPECT_EQ(0, l4_error(f->create(rm.get())));
  EXPECT_EQ(-L4_ENOENT, rm->free_area(0x123995));
}

/**
 * Requesting a factory yields a factory object.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, CreateCheckFactory)
{
  auto f = create_fab();

  auto fab = make_unique_del_cap<L4::Factory>();
  EXPECT_EQ(0, l4_error(f->create(fab.get()) << l4_umword_t(100)));
  auto noob = L4Re::Util::make_unique_del_cap<L4::Thread>();
  EXPECT_EQ(-L4_ENODEV, l4_error(fab->create(noob.get())));
}

/**
 * Requesting a log yields a log object.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, CreateCheckLog)
{
  auto f = create_fab();

  auto l = make_unique_del_cap<L4::Vcon>();
  EXPECT_EQ(0, l4_error(f->create(l.get())
                        << "foo"
                        << l4_umword_t(7)));
  char buf[10] = "123456789";
  EXPECT_EQ(9, l->write(buf, 9));
}

/**
 * Requesting a scheduler is not supported on user-created factories.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, DISABLED_CreateCheckScheduler)
{
  auto f = create_fab();

  auto s = make_unique_del_cap<L4::Scheduler>();
  EXPECT_EQ(0, l4_error(f->create(s.get(), L4::Scheduler::Protocol)));
  EXPECT_FALSE(s->is_online(123456));
}

/**
 * Requesting a dataspace yields a dataspace object.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, CreateCheckDataspace)
{
  auto f = create_fab();

  auto ds = make_unique_del_cap<L4Re::Dataspace>();
  EXPECT_EQ(0, l4_error(f->create(ds.get())
                        << l4_umword_t(L4_PAGESIZE)
                        << l4_umword_t(0)
                        << l4_umword_t(L4_PAGESIZE)));
  EXPECT_EQ(L4_PAGESIZE, unsigned(ds->size()));
}

/**
 * Requesting a DMA space yields a DMA space object.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, CreateCheckDmaspace)
{
  auto f = create_fab();

  auto ds = make_unique_del_cap<L4Re::Dma_space>();
  EXPECT_EQ(0, l4_error(f->create(ds.get())));
  EXPECT_EQ(0, ds->associate(env->task(), L4Re::Dma_space::Phys_space));
}

/**
 * Requesting any kernel object type fails.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, NotASystemFactory)
{
  auto f = create_fab();

  auto dummy = make_unique_cap<void>();
  l4_fpage_t dummyfpage;
  EXPECT_EQ(-L4_ENODEV,
            l4_error(f->create_task(L4::cap_cast<L4::Task>(dummy.get()),
                                    dummyfpage)));
  EXPECT_EQ(-L4_ENODEV,
            l4_error(f->create(L4::cap_cast<L4::Thread>(dummy.get()))));
  EXPECT_EQ(-L4_ENODEV,
            l4_error(f->create_gate(L4::cap_cast<L4::Task>(dummy.get()),
                                    env->main_thread(), 0)));
  EXPECT_EQ(-L4_ENODEV,
            l4_error(f->create(L4::cap_cast<L4::Irq>(dummy.get()))));
  EXPECT_EQ(-L4_ENODEV,
            l4_error(f->create(L4::cap_cast<L4::Vm>(dummy.get()))));
}

/**
 * A factory may not be created without any resource quota.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, ZeroLimits)
{
  auto fab = make_unique_cap<L4::Factory>();
  EXPECT_EQ(-L4_EINVAL,
            l4_error(env->user_factory()->create_factory(fab.get(), 0)));
}

/**
 * Deleting a factory also deletes everything that was created by the
 * factory.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, DeleteRecursively)
{
  auto ns = make_unique_del_cap<L4Re::Namespace>();

    {
      auto f = create_fab(10 * L4_PAGESIZE);
      EXPECT_EQ(0, l4_error(f->create(ns.get(), L4Re::Namespace::Protocol)));
      EXPECT_EQ(0, ns->register_obj("foobar", env->rm()));
    }

  auto dummy2 = make_unique_cap<L4Re::Dataspace>();
  auto ret = ns->query("foobar", dummy2.get());
  EXPECT_TRUE(ret < -L4_EIPC_LO || ret == -L4_EBADPROTO);
}

/**
 * A factory cannot be created with an equal or bigger quota than the factory has
 * that is creating it or with a quota of 0.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, InheritLimits)
{
  auto f = create_fab(10 * L4_PAGESIZE);

  auto fab = make_unique_del_cap<L4::Factory>();
  EXPECT_EQ(-L4_ENOMEM, l4_error(f->create(fab.get(), L4::Factory::Protocol)
                                 << l4_umword_t(20 * L4_PAGESIZE)));
  EXPECT_EQ(-L4_ENOMEM, l4_error(f->create(fab.get(), L4::Factory::Protocol)
                                 << l4_umword_t(10 * L4_PAGESIZE)));
  EXPECT_EQ(-L4_EINVAL, l4_error(f->create(fab.get(), L4::Factory::Protocol)
                                 << l4_umword_t(0)));
}

/**
 * A factory returns its quota to the creator after being deleted.
 *
 * Create a factory from a quota-restrained factory and then confirm
 * that a second factory cannot be created due to being out of memory.
 * Delete the first factory and confirm that the second factory now
 * can be created.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, ReturnQuotaAfterDelete)
{
  auto f = create_fab(10 * L4_PAGESIZE);

  auto fab2 = make_unique_del_cap<L4::Factory>();

  auto fab = make_unique_cap<L4::Factory>();
  EXPECT_EQ(0, l4_error(f->create(fab.get(), L4::Factory::Protocol)
                                  << l4_umword_t(5 * L4_PAGESIZE)));
  EXPECT_EQ(-L4_ENOMEM, l4_error(f->create(fab2.get(), L4::Factory::Protocol)
                                  << l4_umword_t(5 * L4_PAGESIZE)));
  ASSERT_EQ(L4_EOK, l4_error(env->task()->delete_obj(fab.get())));

  EXPECT_EQ(0, l4_error(f->create(fab2.get(), L4::Factory::Protocol)
                                  << l4_umword_t(5 * L4_PAGESIZE)));
}

/**
 * Multiple factories can be used in parallel.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, UseInterleaved)
{
  auto f1 = create_fab(10 * L4_PAGESIZE);
  auto f2 = create_fab(10 * L4_PAGESIZE);

  auto ds1 = create_ds(0, L4_PAGESIZE, L4::cap_cast<L4Re::Mem_alloc>(f1.get()));
  auto ds2 = create_ds(0, L4_PAGESIZE, L4::cap_cast<L4Re::Mem_alloc>(f2.get()));

  auto ns1 = create_ns(f1.get());
  auto ns2 = create_ns(f2.get());

  auto rm1 = create_rm(f1.get());
  auto rm2 = create_rm(f2.get());
}

/**
 * When a factory is deleted, its allocated memory is freed.
 *
 * The test allocates factories until moe reports to be out of memory
 * and then deletes exactly one factory. If the memory was freed correctly
 * it should be possible to allocate a new factory.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, ExhaustQuotaCreate)
{
  auto base = create_fab(2 * L4_PAGESIZE);

  std::vector<L4Re::Util::Ref_cap<L4::Factory>::Cap> fablist;

  for (;;)
    {
      auto fab = make_ref_cap<L4::Factory>();

      long ret = l4_error(base->create_factory(fab.get(), L4_PAGESIZE));
      if (ret == L4_EOK)
        fablist.push_back(fab);
      else
        {
          ASSERT_EQ(-L4_ENOMEM, ret);
          ASSERT_FALSE(fablist.empty());
          // free the previously allocated factory to free memory
          fablist.pop_back();
          break;
        }
    }

  // after freeing, we should be able to get more memory
  auto fab = make_ref_cap<L4::Factory>();
  ASSERT_EQ(L4_EOK, l4_error(base->create_factory(fab.get(), L4_PAGESIZE)));
}

/**
 * When a recursively allocated factory is deleted, its allocated memory is
 * freed.
 *
 * The test allocates factories recursively until moe reports to be out of memory
 * and then deletes exactly one factory. If the memory was freed correctly
 * it should be possible to allocate a new factory.
 *
 * \see L4::Factory.create
 */
TEST_F(TestFactory, ExhaustQuotaCreateRecursive)
{

  std::vector<L4Re::Util::Ref_cap<L4::Factory>::Cap> fablist;

  int pages = 100;
  auto parent = env->user_factory();
  for (;;)
    {
      auto fab = make_ref_cap<L4::Factory>();

      long ret = l4_error(parent->create_factory(fab.get(), pages * L4_PAGESIZE));
      if (ret == L4_EOK)
        {
          fablist.push_back(fab);
          parent = fab.get();
          pages -= 2;
        }
      else
        {
          ASSERT_EQ(-L4_EINVAL, ret);
          ASSERT_TRUE(fablist.size() > 1);
          // free the previously allocated factory to free memory
          fablist.pop_back();
          break;
        }
    }

  // after freeing, we should be able to get more memory
  auto fab = make_ref_cap<L4::Factory>();
  ASSERT_EQ(L4_EOK, l4_error(env->user_factory()->create_factory(fab.get(), L4_PAGESIZE)));
}
