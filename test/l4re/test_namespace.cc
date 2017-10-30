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
    cap = L4Re::chkcap(L4Re::Util::make_unique_cap<L4Re::Namespace>());
  }

  long query(char const *name)
  { return scap()->query(name, cap.get()); }

  L4::Cap<Name_space::Interface> scap() const
  { return handler.obj_cap(); }

  L4Re::Util::Unique_cap<L4Re::Namespace> cap;
  Name_space handler;
};

// Name space tested against local and mapped capabilities
struct NamespaceSvr
: Namespace_fixture,
  ::testing::WithParamInterface<unsigned long>
{
  NamespaceSvr() : Namespace_fixture(GetParam()) {}
};

/**
 * All tests run with two server configurations: first with
 * L4_RCV_ITEM_LOCAL_ID, where the server requests to receive local capability
 * indices whenever possible; second with always receiving newly mapped
 * capabilities.
 */
static unsigned long const receive_cap_flags[] = { L4_RCV_ITEM_LOCAL_ID, 0 };
static INSTANTIATE_TEST_CASE_P(LocalVsGlobal, NamespaceSvr,
                               ::testing::ValuesIn(receive_cap_flags));

/**
 * query returns -L4_ENOENT for names not in the namespace and -L4_EINVAL for
 * invalid names.
 *
 * \see L4Re::Namespace.query
 */
TEST_P(NamespaceSvr, QueryEmptyNameSpace)
{
  EXPECT_EQ(-L4_ENOENT, query("foo"));
  EXPECT_EQ(-L4_ENOENT, query("/foo"));
  EXPECT_EQ(-L4_ENOENT, query("foo/bar"));
  EXPECT_EQ(-L4_EINVAL, query(""));
}

/**
 * An object can be registered with a namespace service and be queried under
 * the registered name afterwards.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST_P(NamespaceSvr, RegisterValid)
{
  handler.reset_counters();
  ASSERT_EQ(-L4_ENOENT, query("bar"));
  ASSERT_EQ(L4_EOK, scap()->register_obj("bar", L4::Ipc::make_cap_rws(scap())));
  ASSERT_EQ(L4_EOK, query("bar"));
  ASSERT_EQ(GetParam()?1:0, handler.reserve_epiface_count);
  ASSERT_EQ(GetParam()?0:1, handler.reserve_cap_count);
}

/**
 * Multiple names can be registered with a namespace service.
 *
 * \see L4Re::Namespace.register_obj
 */
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

/**
 * A name can be reserved with a namespace service when an invalid capability
 * is registered. A valid capability can be registered later for such a
 * reserved entry.
 *
 * \see L4Re::Namespace.register_obj
 */
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

/**
 * A name reserved in a namespace service cannot be removed.
 *
 * \see L4Re::Namespace.unlink
 */
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

/**
 * An object cannot be registered with a name containing slashes.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST_P(NamespaceSvr, RegisterWithSlash)
{
  ASSERT_EQ(-L4_EINVAL, scap()->register_obj("ping/pong", scap()));
}

/**
 * An object cannot be registered with an empty name.
 *
 * \see L4Re::Namespace.register_obj
 */
TEST_P(NamespaceSvr, RegisterEmpty)
{
  ASSERT_EQ(-L4_EINVAL, scap()->register_obj("", scap()));
}

/**
 * An existing name with a valid object registration cannot be overwritten with
 * an invalid capability without using the L4Re::Namespace::Overwrite flag.
 *
 * \see L4Re::Namespace.register_obj
 */
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

// The following tests are only performed with newly mapped
// capabilities. The test RegisterPropagateRights can only run with
// newly mapped capabilities.
struct MappedNamespaceSvr : Namespace_fixture
{
  MappedNamespaceSvr() : Namespace_fixture(0) {}
};

/**
 * When a client possesses only a read-only capability to an object it cannot
 * register the capability with read-write rights.
 *
 * Note that query() is a member function in Namespace_fixture that
 * saves the retrieved capability slot in the `cap` member.
 *
 * \see L4Re::namespace.register_obj
 */
TEST_F(MappedNamespaceSvr, RegisterPropagateRights)
{
  // Register capability read-only.
  ASSERT_EQ(L4_EOK, scap()->register_obj("first", scap(), L4Re::Namespace::Ro));
  ASSERT_EQ(L4_EOK, query("first"));
  ASSERT_TRUE(cap.is_valid());
  // We should not be allowed to register on the returned capability.
  ASSERT_EQ(-L4_EPERM, cap->register_obj("foo", L4::Cap<void>()));
  // Register the previously queried cap under a new name with full rights.
  ASSERT_EQ(L4_EOK, scap()->register_obj("second",
                                         L4::Ipc::make_cap_rw(cap.get()),
                                         L4Re::Namespace::Rw));
  ASSERT_EQ(L4_EOK, query("second"));
  // The second cap is missing the write permission.
  ASSERT_EQ(-L4_EPERM, cap->register_obj("foo", L4::Cap<void>()));
}

/**
 * An object registered with a namespace service under a specific name is
 * overwritten with a new object under the same name, if the
 * L4Re::Namespace::Overwrite flag is specified.
 *
 * \see L4Re::namespace.register_obj
 */
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
  auto cap2 = L4Re::Util::make_unique_cap<L4Re::Dataspace>();
  ASSERT_EQ(L4_EOK, cap->query("l4re", cap2.get()));
}

/**
 * A registered name is removed from the namespace service when the object
 * registered under the name is deleted.
 *
 * \see L4Re::namespace.register_obj
 */
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
  // The namespace server only notes that the capability is gone
  // during the query() call.
  ASSERT_EQ(0, handler.free_cap_count);
  EXPECT_LE(ds.validate(L4_BASE_TASK_CAP).label(), 0);
  EXPECT_EQ(-L4_ENOENT, query("gone"));
  ASSERT_EQ(1, handler.free_cap_count);

  EXPECT_EQ(true, L4Re::Util::cap_alloc.free(ds));
}

/**
 * Namespaces are resolved recursively, when the query contains multiple
 * elements separated by slash
 *
 * \see L4Re::namespace.register_obj
 */
TEST_F(MappedNamespaceSvr, Recursive)
{
  ASSERT_EQ(0, scap()->register_obj("ping", scap()));
  ASSERT_EQ(0, scap()->register_obj("pong", scap()));
  ASSERT_EQ(0, scap()->register_obj("r", env->get_cap<L4Re::Namespace>("rom")));

  auto tcap = L4Re::Util::make_unique_cap<L4Re::Namespace>();

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
