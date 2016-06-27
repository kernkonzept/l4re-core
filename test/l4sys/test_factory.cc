/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * Unit tests for the following factory class functions:
 *  create<OBJ>()
 *  create_factory()
 *  create_thread()
 *  create_task()
 *  create_irq()
 *  create_gate()
 *
 *  TODO validate existence/absence of kernel object through usage.
 */
#include <l4/sys/capability>
#include <l4/sys/factory>
#include <l4/sys/icu>
#include <l4/sys/ipc_gate>
#include <l4/sys/irq>
#include <l4/sys/scheduler>
#include <l4/sys/task>
#include <l4/sys/thread>
#include <l4/sys/err.h>
#include <l4/sys/ipc.h>
#include <l4/sys/kobject>
#include <l4/sys/meta>
#include <l4/sys/smart_capability>
#include <l4/sys/vcon>

#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>

#include <l4/atkins/debug>
#include <l4/atkins/factory>
#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>

#include "arch_config.h"

static auto factory = L4Re::Env::env()->factory();

/**
 * Type-parameterised test fixture for create<T> function.
 */
template <typename T>
class FactoryCreateObjTest : public testing::Test
{
protected:
  l4_msgtag_t create(L4::Cap<T> cap) { return factory->create(cap); }
};

template <>
l4_msgtag_t
FactoryCreateObjTest<L4::Task>::create(L4::Cap<L4::Task> cap)
{
  return factory->create(cap) << l4_fpage_invalid();
}

template <>
l4_msgtag_t
FactoryCreateObjTest<L4::Factory>::create(L4::Cap<L4::Factory> cap)
{
  return factory->create(cap) << -1UL;
}

template <>
l4_msgtag_t
FactoryCreateObjTest<L4::Ipc_gate>::create(L4::Cap<L4::Ipc_gate> cap)
{
  return factory->create_gate(cap, L4Re::Env::env()->main_thread(), -1);
}

typedef testing::Types<L4::Factory, L4::Icu, L4::Ipc_gate, L4::Irq, L4::Task,
                       L4::Thread>
  ObjectTypes;

TYPED_TEST_CASE(FactoryCreateObjTest, ObjectTypes);

/**
 * Type-parameterised tests to test kernel object creation of the types stated
 * in TYPED_TEST_CASE above.
 * This tests the normal case and is expected to succeed. Actual object
 * creation is not validated by usage.
 */
TYPED_TEST(FactoryCreateObjTest, CreateObj)
{
  auto tcap = Atkins::Factory::cap<TypeParam>();
  ASSERT_L4OK(this->create(tcap.get())) << "Protocol: " << TypeParam::Protocol;
}

/**
 * Value-parameterized fixture to test the memory limit for newly created
 * factories.
 */
class CreateFactoryTest : public testing::TestWithParam<l4_umword_t>
{
};

static INSTANTIATE_TEST_CASE_P(FactoryLimit, CreateFactoryTest,
                               testing::Values(0UL, 1UL, ~0UL, 3 * L4_PAGESIZE));

/**
 * Value-parameterized test for memory limitations defined in the
 * INSTANTIATE_TEST_CASE_P above.
 */
TEST_P(CreateFactoryTest, ValidCapLimitUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Factory>();
  ASSERT_L4OK(factory->create_factory(cap.get(), GetParam(), l4_utcb()));
}

//**************************************************************************

/**
 * Fixture to group create_thread tests.
 */
class CreateThreadTest : public testing::Test
{
};

/**
 * Tests the creation of a kernel thread object through the factory.
 * Validates the existence of the kernel object by calling ex_regs on the
 * object.
 */
TEST_F(CreateThreadTest, ValidCapUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Thread>();
  ASSERT_L4OK(factory->create_thread(cap.get(), l4_utcb()));
  ASSERT_L4OK(cap->ex_regs(~0, ~0, 0));
}

//**************************************************************************

/**
 * Fixture to group create_task tests with rights value-parameterization.
 */
class CreateTaskTest : public testing::TestWithParam<unsigned int>
{
protected:
  l4_fpage_t create_fpage(unsigned sz = L4_LOG2_PAGESIZE,
                          l4_addr_t a = _fpaddr,
                          L4_fpage_rights r = L4_FPAGE_RW)
  {
    return l4_fpage(a, sz, r);
  }

  static const l4_addr_t _fpaddr = 0x100000UL;
};

static INSTANTIATE_TEST_CASE_P(CreateTaskFpages, CreateTaskTest,
                               testing::Values(0, L4_FPAGE_X, L4_FPAGE_W,
                                               L4_FPAGE_RO, L4_FPAGE_RW,
                                               L4_FPAGE_RX, L4_FPAGE_RWX,
                                               L4_FPAGE_X | L4_FPAGE_W));

/**
 * Value-parameterized test for task creation testing rights values for the
 * passed flex-page.
 */
TEST_P(CreateTaskTest, ValidCapRightsFpageUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4OK(
    factory->create_task(cap.get(),
                         l4_fpage(_fpaddr, L4_LOG2_PAGESIZE, GetParam()),
                         l4_utcb()));
}

/**
 * Task kernel object creation test.
 */
TEST_F(CreateTaskTest, NormalCase)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4OK(
    factory->create_task(cap.get(), create_fpage(L4_LOG2_PAGESIZE), l4_utcb()));
}

/**
 * Task kernel object creation test.
 * Expect to fail for a flex-page smaller than L4_PAGESIZE;
 */
TEST_F(CreateTaskTest, HalfFpageSize)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4ERR(L4_EINVAL,
               factory->create_task(cap.get(),
                                    create_fpage(L4_LOG2_PAGESIZE - 1),
                                    l4_utcb()));
}

/**
 * Task kernel object creation test.
 * Expect to succeed for a flex-page larger than L4_PAGESIZE;
 */
TEST_F(CreateTaskTest, TwiceFpageSize)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4OK(factory->create_task(cap.get(), create_fpage(L4_LOG2_PAGESIZE + 1),
                                   l4_utcb()));
}

/**
 * Task kernel object creation test.
 * Expected to fail, as the requested memory address is in the kernel reserved
 * area of the address space.
 */
TEST_F(CreateTaskTest, KernelMemoryAreaFpage)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  unsigned sz = L4_LOG2_PAGESIZE;
  ASSERT_L4ERR(L4_EEXIST,
               factory
                 ->create_task(cap.get(),
                               create_fpage(sz,
                                            Arch_config::Kernel_memory_address),
                               l4_utcb()));
}

/**
 * Task kernel object creation test.
 * Expected to succeed, with a task object missing valid kernel-user memory.
 */
TEST_F(CreateTaskTest, ValidCapInvFpageUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4OK(factory->create_task(cap.get(), l4_fpage_invalid(), l4_utcb()));
}

/**
 * Task kernel object creation test.
 * Expected to fail, as the size of the requested address space is too large
 * for kernel's alloc_ku_mem.
 */
TEST_F(CreateTaskTest, ValidCapAllFpageUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4ERR(L4_EINVAL,
               factory->create_task(cap.get(), l4_fpage_all(), l4_utcb()));
}

//**************************************************************************

/**
 * Fixture to group create_irq tests.
 */
class CreateIrqTest : public testing::Test
{
};
/**
 * Irq kernel object creation test. Expected to succeed.
 */
TEST_F(CreateIrqTest, ValidCapUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Irq>();
  ASSERT_L4OK(factory->create_irq(cap.get(), l4_utcb()));
}

//**************************************************************************

/**
 * Value-parameterized fixture to group create Ipc_gate tests and test
 * different Ipc_gate labels.
 */
class CreateGateTest : public testing::TestWithParam<l4_umword_t>
{
};

static INSTANTIATE_TEST_CASE_P(CreateGateTestInstances, CreateGateTest,
                               testing::Values(0UL, -1UL));

/**
 * Create Ipc_gate kernel object and attach it to this thread.
 */
TEST_P(CreateGateTest, ValidCapValidThreadLabelsUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  ASSERT_L4OK(factory->create_gate(cap.get(), L4Re::Env::env()->main_thread(),
                                   (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object and attach it to a valid capability selector
 * with which no thread was created.
 * Expected to fail, as the thread the Ipc_gate shall be bound to, does not
 * exist.
 */
TEST_P(CreateGateTest, ValidCapValidEmptyThreadCapLabelsUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto thread_cap = Atkins::Factory::cap<L4::Thread>();
  ASSERT_TRUE(cap.is_valid());
  ASSERT_L4ERR(L4_ENOENT,
               factory->create_gate(cap.get(), thread_cap.get(),
                                    (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object and attach it to an existing Task object,
 * whose user-space capability was cast to L4::Thread.
 * Expect to fail, as the type of the kernel object is wrong.
 */
TEST_P(CreateGateTest, KernelCreateGateWithTaskObjCastToThread)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto task_cap = Atkins::Factory::cap<L4::Task>();
  ASSERT_L4OK(factory->create_task(task_cap.get(), l4_fpage_invalid()));

  auto now_thread_cap = L4::cap_reinterpret_cast<L4::Thread>(task_cap);
  ASSERT_L4ERR(L4_ENOENT,
               factory->create_gate(cap.get(), now_thread_cap.get(),
                                    (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object and attach it to an existing Factory object,
 * whose user-space capability was cast to L4::Thread.
 * Expect to fail, as the type of the kernel object is wrong.
 */
TEST_P(CreateGateTest, KernelCreateGateWithFactoryObjCastToThread)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto factory_cap = Atkins::Factory::cap<L4::Factory>();
  ASSERT_L4OK(factory->create_factory(factory_cap.get(), 0UL));

  auto now_thread_cap = L4::cap_reinterpret_cast<L4::Thread>(factory_cap);
  ASSERT_L4ERR(L4_ENOENT,
               factory->create_gate(cap.get(), now_thread_cap.get(),
                                    (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object and attach it to an existing Ipc_gate object,
 * whose user-space capability was cast to L4::Thread.
 * Expect to fail, as the type of the kernel object is wrong.
 */
TEST_P(CreateGateTest, KernelCreateGateWithIpcGateObjCastToThread)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto kobj_cap = Atkins::Factory::cap<L4::Ipc_gate>();
  ASSERT_L4OK(factory->create_gate(kobj_cap.get(),
                                   L4Re::Env::env()->main_thread(), 0xaffeUL));

  auto now_thread_cap = L4::cap_reinterpret_cast<L4::Thread>(kobj_cap);
  ASSERT_L4ERR(L4_ENOENT,
               factory->create_gate(cap.get(), now_thread_cap.get(),
                                    (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object and attach it to an existing Irq object, whose
 * user-space capability was cast to L4::Thread.
 * Expect to fail, as the type of the kernel object is wrong.
 */
TEST_P(CreateGateTest, KernelCreateGateWithIrqObjCastToThread)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto irq_cap = Atkins::Factory::cap<L4::Irq>();
  ASSERT_L4OK(factory->create_irq(irq_cap.get()));

  auto now_thread_cap = L4::cap_reinterpret_cast<L4::Thread>(irq_cap);
  ASSERT_L4ERR(L4_ENOENT,
               factory->create_gate(cap.get(), now_thread_cap.get(),
                                    (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object and attach it to an existing Task object,
 * whose user-space capability was cast to L4::Thread.
 * Expect to fail, as the type of the kernel object is wrong.
 * Validate the absence of the Ipc_gate object, by sending an Ipc to it.
 */
TEST_P(CreateGateTest, KernelCreateGateWithTaskObjCastToThreadAndUseGate)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto task_cap = Atkins::Factory::cap<L4::Task>();
  EXPECT_L4OK(factory->create_task(task_cap.get(), l4_fpage_invalid()));

  auto now_thread_cap = L4::cap_reinterpret_cast<L4::Thread>(task_cap);
  EXPECT_L4ERR(L4_ENOENT,
               factory->create_gate(cap.get(), now_thread_cap.get(),
                                    (l4_umword_t)GetParam(), l4_utcb()));

  l4_timeout_t to = l4_ipc_timeout(10, 10, 10, 10);
  auto ret = l4_ipc_call(cap.get().cap(), l4_utcb(), l4_msgtag(0, 0, 0, 0), to);
  ASSERT_L4ERR(L4_IPC_ENOT_EXISTENT + L4_EIPC_LO, ret);
}

/**
 * Create Ipc_gate kernel object not bound to any thread.
 */
TEST_P(CreateGateTest, ValidCapInvalidEmptyThreadCapLabelsUtcb)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto empty_cap_slot = L4::Cap<L4::Thread>();
  ASSERT_L4OK(factory->create_gate(cap.get(), empty_cap_slot,
                                   (l4_umword_t)GetParam(), l4_utcb()));
}

/**
 * Create Ipc_gate kernel object not bound to any thread.
 * Validate the behaviour by sending an Ipc to the gate, which times out, as
 * there is no thread to forward the Ipc to.
 */
TEST_P(CreateGateTest, ValidCapEmptyThreadCapLabelsUtcbSendToEmpty)
{
  auto cap = Atkins::Factory::cap<L4::Ipc_gate>();
  auto empty_cap_slot = L4::Cap<L4::Thread>();
  ASSERT_L4OK(factory->create_gate(cap.get(), empty_cap_slot,
                                   (l4_umword_t)GetParam(), l4_utcb()));

  l4_timeout_t to = l4_ipc_timeout(10, 10, 10, 10);
  auto ret = l4_ipc_call(cap.get().cap(), l4_utcb(), l4_msgtag(0, 0, 0, 0), to);
  ASSERT_L4ERR(L4_IPC_SETIMEOUT + L4_EIPC_LO, ret);
}

/**
 * Create Ipc_gate kernel object and pass a capability on a thread object
 * without write rights.
 * Expect to fail, as rights are missing.
 */
TEST_P(CreateGateTest, NoWriteRightsThreadCap)
{
  TODO("Trigger L4_EPERM return value by passing a thread_cap with no write rights.");
  FAIL();
}
