/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Marcus Hähnel <marcus.haehnel@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * \file
 *
 * Tests for the RPC meta protocol.
 */

#include <l4/sys/factory>

#include <l4/re/env>
#include <l4/re/util/unique_cap>
#include <l4/re/error_helper>
#include <l4/re/namespace>
#include <l4/re/dataspace>
#include <l4/re/dma_space>
#include <l4/re/mem_alloc>
#include <l4/re/rm>

#include <l4/atkins/tap/main>

static L4Re::Env const *const env = L4Re::Env::env();

/**
 * Allocator functions for the different capabilities.
 *
 * This template covers all types that do not need special arguments upon
 * creation.
 */
template <typename T>
static l4_msgtag_t
alloc_cap(L4::Cap<T> cap)
{
  return env->mem_alloc()->create(cap);
}

template <>
l4_msgtag_t
alloc_cap(L4::Cap<L4Re::Dataspace> cap)
{
  return env->mem_alloc()->create(cap, L4Re::Dataspace::Protocol)
         << l4_umword_t(L4_PAGESIZE) // Size
         << l4_umword_t(0)           // Flags
         << l4_umword_t(0);          // Alignment
}

template <>
l4_msgtag_t
alloc_cap(L4::Cap<L4Re::Mem_alloc> cap)
{
  return env->mem_alloc()->create_factory(cap, 10 * L4_PAGESIZE);
}

template <>
l4_msgtag_t
alloc_cap(L4::Cap<L4::Factory> cap)
{
  return env->mem_alloc()->create_factory(cap, 10 * L4_PAGESIZE);
}

/**
 * Type-parameterised test fixture
 *
 * This fixture allocates a capability for the template type and calls the
 * corresponding factory to create an object of that type. It then provides a
 * capability for speaking the Meta protocol with this object through the `_cap`
 * member.
 */
template <typename T>
class RPCMeta : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<T>(),
                        "Capability allocated successfully");
    L4Re::chksys(alloc_cap(_cap.get()), "Object created successfully");
    _meta = L4::cap_reinterpret_cast<L4::Meta>(_cap.get());
  }

protected:
  L4Re::Util::Unique_del_cap<T> _cap;
  L4::Cap<L4::Meta> _meta;
};

typedef testing::Types<L4Re::Namespace, L4Re::Dataspace, L4Re::Dma_space,
                       L4Re::Rm, L4Re::Mem_alloc, L4::Factory>
  TestTypes;

TYPED_TEST_CASE(RPCMeta, TestTypes);

/**
 * RPC interfaces for an object support at least one interface.
 */
TYPED_TEST(RPCMeta, NumInterfaces)
{
  ASSERT_LE(1, this->_meta->num_interfaces().label())
    << "At least one interface is supported by the IPC partner";
}

/**
 * The RPC interface of an object supports the interface for which the
 * capability was created.
 */
TYPED_TEST(RPCMeta, SupportsIface)
{
  ASSERT_EQ(this->_meta->supports(TypeParam::Protocol).label(), 1)
    << "The object speaks the interface for which the capability was created.";
}

/**
 * The string description of an interface is non-empty.
 */
TYPED_TEST(RPCMeta, FirstInterface)
{
  long proto;
  char namec[256] = {0};
  L4::Ipc::String<char> name(sizeof(namec), namec);
  EXPECT_EQ(this->_meta->interface(0, &proto, &name).label(), 0)
    << "Information of interface 0 can be queried.";
  ASSERT_GT(strlen(namec), 0ul)
    << "A string description of the interface can be obtained.";
}

