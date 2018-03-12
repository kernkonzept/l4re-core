/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test namespace interface and reference implementation in l4/re/util.
 */

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

#include <l4/re/env>

#include "namespace_impl.h"

static L4Re::Env const * const env = L4Re::Env::env();

/*
 * General fixture that starts a server thread with the
 * simple name space handler.
 */
struct Namespace_fixture : Atkins::Fixture::Server_thread
{
  Namespace_fixture(unsigned long cap_flags)
  { server.set_rcv_cap_flags(cap_flags); }

  virtual void SetUp()
  {
    L4Re::chkcap(server.registry()->register_obj(&handler));
    start_loop();

    handler.registry = server.registry();
    cap = L4Re::chkcap(L4Re::Util::cap_alloc.alloc<L4Re::Namespace>());
  }

  long query(char const *name)
  { return scap()->query(name, cap.get()); }

  L4::Cap<Name_space::Interface> scap() const
  { return handler.obj_cap(); }

  L4Re::Util::Auto_cap<L4Re::Namespace>::Cap cap;
  Name_space handler;
};

// Name space tested against local and mapped capabilities
struct NamespaceSvr
: Namespace_fixture,
  ::testing::WithParamInterface<unsigned long>
{
  NamespaceSvr() : Namespace_fixture(GetParam()) {}
};

static INSTANTIATE_TEST_CASE_P(LocalVsGlobal, NamespaceSvr,
                               ::testing::Values(L4_RCV_ITEM_LOCAL_ID, 0));


TEST_P(NamespaceSvr, QueryEmptyNameSpace)
{
  EXPECT_EQ(-L4_ENOENT, query("foo"));
  EXPECT_EQ(-L4_ENOENT, query("/foo"));
  EXPECT_EQ(-L4_ENOENT, query("foo/bar"));
  EXPECT_EQ(-L4_EINVAL, query(""));
}

TEST_P(NamespaceSvr, RegisterValid)
{
  handler.reset_counters();
  ASSERT_EQ(-L4_ENOENT, query("bar"));
  ASSERT_EQ(L4_EOK, scap()->register_obj("bar", L4::Ipc::make_cap_rws(scap())));
  ASSERT_EQ(L4_EOK, query("bar"));
  ASSERT_EQ(GetParam()?1:0, handler.reserve_epiface_count);
  ASSERT_EQ(GetParam()?0:1, handler.reserve_cap_count);
}

TEST_P(NamespaceSvr, RegisterMultipleValid)
{
  handler.reset_counters();
  ASSERT_EQ(L4_EOK, scap()->register_obj("something",
                                         L4::Ipc::make_cap_rws(scap())));
  ASSERT_EQ(L4_EOK, scap()->register_obj("nothing", L4::Cap<void>()));
  ASSERT_EQ(L4_EOK, query("something"));
  ASSERT_EQ(GetParam()?1:0, handler.reserve_epiface_count);
  ASSERT_EQ(GetParam()?0:1, handler.reserve_cap_count);
}

TEST_P(NamespaceSvr, RegisterInvalid)
{
  ASSERT_EQ(-L4_ENOENT, query("pend"));
  // Register just the name.
  ASSERT_EQ(0, scap()->register_obj("pend", L4::Cap<void>()));
  // Server should tell us to try again later.
  ASSERT_EQ(-L4_EAGAIN, scap()->query("pend", cap.get(),
                                      L4Re::Namespace::To_non_blocking));
  // Now register the correct one.
  ASSERT_EQ(0, scap()->register_obj("pend", scap()));
  // And we should get back the capability.
  ASSERT_EQ(0, query("pend"));
}

TEST_P(NamespaceSvr, FreeInvalidEntry)
{
  handler.reset_counters();
  ASSERT_EQ(0, scap()->register_obj("pend", L4::Cap<void>()));
  ASSERT_EQ(-L4_ENOENT, scap()->unlink("pend"));
  ASSERT_EQ(0, handler.reserve_epiface_count);
  ASSERT_EQ(0, handler.reserve_cap_count);
  ASSERT_EQ(0, handler.free_epiface_count);
  ASSERT_EQ(0, handler.free_cap_count);
}

TEST_P(NamespaceSvr, RegisterWithSlash)
{
  ASSERT_EQ(-L4_EINVAL, scap()->register_obj("ping/pong", scap()));
}

TEST_P(NamespaceSvr, RegisterEmpty)
{
  ASSERT_EQ(-L4_EINVAL, scap()->register_obj("", scap()));
}

TEST_P(NamespaceSvr, RegisterOverwriteInvalid)
{
  handler.reset_counters();
  ASSERT_EQ(-L4_ENOENT, query("f"));
  ASSERT_EQ(L4_EOK, scap()->register_obj("f", scap()));
  ASSERT_EQ(GetParam()?1:0, handler.reserve_epiface_count);
  ASSERT_EQ(GetParam()?0:1, handler.reserve_cap_count);
  ASSERT_EQ(L4_EOK, query("f"));
  ASSERT_EQ(-L4_EEXIST, scap()->register_obj("f", L4::Cap<void>()));
  ASSERT_EQ(0, handler.free_epiface_count);
  ASSERT_EQ(0, handler.free_cap_count);
  ASSERT_EQ(L4_EOK, scap()->register_obj("f", L4::Cap<void>(),
                                         L4Re::Namespace::Overwrite |
                                         L4Re::Namespace::Rw));
  ASSERT_EQ(GetParam()?1:0, handler.free_epiface_count);
  ASSERT_EQ(GetParam()?0:1, handler.free_cap_count);
  ASSERT_EQ(-L4_EAGAIN, scap()->query("f", cap.get(),
                                      L4Re::Namespace::To_non_blocking));
}

// Test that can only be run against mapped capabilities.
struct MappedNamespaceSvr : Namespace_fixture
{
  MappedNamespaceSvr() : Namespace_fixture(0) {}
};

TEST_F(MappedNamespaceSvr, RegisterPropagateRights)
{
  // Register capability read-only.
  ASSERT_EQ(L4_EOK, scap()->register_obj("first", scap(), L4Re::Namespace::Ro));
  ASSERT_EQ(L4_EOK, query("first"));
  ASSERT_TRUE(cap.is_valid());
  // We should not be allowed to register on the returned capability.
  ASSERT_EQ(-L4_EPERM, cap->register_obj("foo", L4::Cap<void>()));
  // Register the cap under a new name with full rights.
  ASSERT_EQ(L4_EOK, scap()->register_obj("second",
                                         L4::Ipc::make_cap_rw(cap.get()),
                                         L4Re::Namespace::Rw));
  ASSERT_EQ(L4_EOK, query("second"));
  // We still shouldn't be able to register on that second cap.
  ASSERT_EQ(-L4_EPERM, cap->register_obj("foo", L4::Cap<void>()));
}


TEST_F(MappedNamespaceSvr, RegisterOverwriteNew)
{
  handler.reset_counters();
  ASSERT_EQ(-L4_ENOENT, query("f"));
  ASSERT_EQ(L4_EOK, scap()->register_obj("f", scap()));
  ASSERT_EQ(1, handler.reserve_cap_count);
  ASSERT_EQ(L4_EOK, query("f"));
  auto rom = env->get_cap<L4Re::Namespace>("rom");
  ASSERT_EQ(L4_EOK, rom->query("l4re", cap.get()));
  ASSERT_EQ(-L4_EEXIST, scap()->register_obj("f", rom));
  ASSERT_EQ(2, handler.reserve_cap_count);
  ASSERT_EQ(1, handler.free_cap_count);
  ASSERT_EQ(L4_EOK, scap()->register_obj("f", rom,
                                         L4Re::Namespace::Overwrite |
                                         L4Re::Namespace::Rw));
  ASSERT_EQ(3, handler.reserve_cap_count);
  ASSERT_EQ(2, handler.free_cap_count);
  ASSERT_EQ(L4_EOK, query("f"));
  // check for rom namespace
  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap cap2;
  cap2 = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  ASSERT_EQ(L4_EOK, cap->query("l4re", cap2.get()));
}

TEST_F(MappedNamespaceSvr, RegisterAndUnmapDataspace)
{
  handler.reset_counters();
  auto ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  ASSERT_TRUE(ds.is_valid());
  ASSERT_EQ(L4_EOK, env->mem_alloc()->alloc(L4_PAGESIZE, ds, 0));

  // Start with no entry.
  ASSERT_EQ(-L4_ENOENT, query("gone"));
  // After registering the entry, there should be a response.
  ASSERT_EQ(L4_EOK, scap()->register_obj("gone", ds));
  ASSERT_EQ(1, handler.reserve_cap_count);
  ASSERT_EQ(L4_EOK, query("gone"));
  // Now delete the cap and there should be no entry.
  ASSERT_EQ(L4_EOK, l4_error(env->task()->unmap(ds.fpage(), L4_FP_DELETE_OBJ |
                                                L4_FP_ALL_SPACES)));
  ASSERT_EQ(0, handler.free_cap_count);
  EXPECT_LE(ds.validate(L4_BASE_TASK_CAP).label(), 0);
  EXPECT_EQ(-L4_ENOENT, query("gone"));
  ASSERT_EQ(1, handler.free_cap_count);

  EXPECT_EQ(true, L4Re::Util::cap_alloc.free(ds));
}

TEST_F(MappedNamespaceSvr, Recursive)
{
  ASSERT_EQ(0, scap()->register_obj("ping", scap()));
  ASSERT_EQ(0, scap()->register_obj("pong", scap()));
  ASSERT_EQ(0, scap()->register_obj("r", env->get_cap<L4Re::Namespace>("rom")));

  L4Re::Util::Auto_cap<L4Re::Namespace>::Cap tcap;
  tcap = L4Re::Util::cap_alloc.alloc<L4Re::Namespace>();

  EXPECT_EQ(0, scap()->query("r", tcap.get()));
  ASSERT_EQ(L4_EOK, tcap->query("l4re", cap.get()));

  EXPECT_EQ(0, scap()->query("ping/r", tcap.get()));
  ASSERT_EQ(L4_EOK, tcap->query("l4re", cap.get()));

  EXPECT_EQ(0, scap()->query("pong/r", tcap.get()));
  ASSERT_EQ(L4_EOK, tcap->query("l4re", cap.get()));

  EXPECT_EQ(0, scap()->query("ping/pong/ping/r", tcap.get()));
  ASSERT_EQ(L4_EOK, tcap->query("l4re", cap.get()));

  EXPECT_EQ(0, scap()->query("ping/ping/ping/r", tcap.get()));
  ASSERT_EQ(L4_EOK, tcap->query("l4re", cap.get()));

  EXPECT_EQ(0, scap()->query("ping/r/l4re", tcap.get()));
  EXPECT_EQ(-L4_EBADPROTO, scap()->query("ping/r/l4re/ping", tcap.get()));
}
