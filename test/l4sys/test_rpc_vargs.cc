/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test marshalling and unmarshalling of varargs in RPC.
 */

#include <l4/sys/cxx/ipc_iface>
#include <l4/sys/cxx/ipc_varg>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, in, (L4::Ipc::Varg const *args));

  typedef L4::Typeid::Rpcs<in_t> Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  Test_handler() : p_args(L4::Ipc::Varg_list_ref()) {}

  long op_in(Test_iface::Rights, L4::Ipc::Varg_list_ref &args)
  {
    p_args = args;
    return 0;
  }

  L4::Ipc::Varg_list<> p_args;
};

using Varg = L4::Ipc::Varg;

struct VargRPC : Atkins::Fixture::Epiface_thread<Test_handler>
{
  void cmp_handler_list(Varg const *in)
  {
    L4::Ipc::Varg_list<> &vlist = handler().p_args;

    for (; in->tag(); ++in)
      {
        Varg o = vlist.next();
        ASSERT_EQ(in->tag(), o.tag());
        EXPECT_EQ(0, memcmp(in->data(), o.data(), in->length()));
      }

    EXPECT_EQ(L4_VARG_TYPE_NIL, vlist.next().tag());
  }
};

/**
 * A variable argument parameter without content is transferred correctly.
 */
TEST_F(VargRPC, InEmpty)
{
  Varg v = Varg::nil();
  ASSERT_EQ(0, scap()->in(&v));
  cmp_handler_list(&v);
}

/**
 * A variable argument parameter sending a signed word as reference
 * is transferred correctly.
 */
TEST_F(VargRPC, InSimpleMword)
{
  l4_mword_t const m = -0x453;
  Varg v[2] = {Varg(&m), Varg::nil()};
  ASSERT_EQ(0, scap()->in(v));
  Varg out = handler().p_args.next();
  ASSERT_TRUE(out.is_of<l4_mword_t>());
  ASSERT_EQ(m, out.value<l4_mword_t>());
  ASSERT_EQ(sizeof(l4_mword_t), (size_t) out.length());
  ASSERT_FALSE(out.is_nil());
}

/**
 * A variable argument parameter sending an unsigned word as reference
 * is transferred correctly.
 */
TEST_F(VargRPC, InSimpleUmword)
{
  l4_umword_t const m = 0xaaa;
  Varg v[2] = {Varg(&m), Varg::nil()};
  ASSERT_EQ(0, scap()->in(v));
  Varg out = handler().p_args.next();
  ASSERT_TRUE(out.is_of<l4_umword_t>());
  ASSERT_EQ(m, out.value<l4_umword_t>());
  ASSERT_EQ(sizeof(l4_umword_t), (size_t) out.length());
  ASSERT_FALSE(out.is_nil());
}

/**
 * A variable argument parameter sending a string is transferred correctly.
 */
TEST_F(VargRPC, InSimpleString)
{
  char const *s = "This ";
  Varg v[2] = {Varg(s), Varg::nil()};
  ASSERT_EQ(0, scap()->in(v));
  Varg out = handler().p_args.next();
  ASSERT_TRUE(out.is_of<char const *>());
  ASSERT_STREQ(s, out.value<char const *>());
  ASSERT_FALSE(out.is_nil());
}

/**
 * A variable argument parameter sending an empty string is transferred correctly.
 */
TEST_F(VargRPC, InEmptyString)
{
  Varg v[2] = {Varg(""), Varg::nil()};
  ASSERT_EQ(0, scap()->in(v));
  Varg out = handler().p_args.next();
  ASSERT_TRUE(out.is_of<char const *>());
  ASSERT_STREQ("", out.value<char const *>());
  ASSERT_FALSE(out.is_nil());
}

/**
 * A variable argument parameter sending a signed word as value
 * is transferred correctly.
 */
TEST_F(VargRPC, InDirectMword)
{
  Varg v[2] = {Varg((l4_mword_t) -0xF3F4), Varg::nil()};
  ASSERT_EQ(0, scap()->in(v));
  Varg out = handler().p_args.next();
  l4_mword_t val;
  ASSERT_TRUE(out.get_value<l4_mword_t>(&val));
  ASSERT_EQ(-0xF3F4, val);
  ASSERT_EQ(sizeof(l4_mword_t), (size_t) out.length());
  ASSERT_FALSE(out.is_nil());
}

/**
 * A variable argument parameter sending an unsigned word as value
 * is transferred correctly.
 */
TEST_F(VargRPC, InDirectUmword)
{
  Varg v[2] = {Varg((l4_umword_t) 1234, true), Varg::nil()};
  ASSERT_EQ(0, scap()->in(v));
  Varg out = handler().p_args.next();
  l4_umword_t val;
  ASSERT_TRUE(out.get_value<l4_umword_t>(&val));
  ASSERT_EQ(1234U, val);
  ASSERT_EQ(sizeof(l4_umword_t), (size_t) out.length());
  ASSERT_FALSE(out.is_nil());
}

/**
 * A variable argument parameter whose content does not fit into the UTCB
 * yields an error.
 */
TEST_F(VargRPC, InOverlongMsg)
{
  Varg v[1000];
  for (l4_umword_t i = 0; i < 999; ++i)
    v[i] = Varg(i);
  v[999] = Varg::nil();
  ASSERT_EQ(-L4_EMSGTOOLONG, scap()->in(v));
}

struct MixedVargRPC : VargRPC, ::testing::WithParamInterface<Varg const *> {};

/**
 * A variable argument parameter with content of mixed types is
 * transferred correctly.
 */
TEST_P(MixedVargRPC, InMixedArgs)
{
  ASSERT_EQ(0, scap()->in(GetParam()));
  cmp_handler_list(GetParam());
}

static Varg const mv1[] = {Varg((l4_mword_t) 1), Varg("foot"),
              Varg(l4_umword_t(34)), Varg(""), Varg::nil()};
static Varg const mv2[] = {Varg("foo"), Varg("foo"), Varg::nil()};

static INSTANTIATE_TEST_CASE_P(Singleton, MixedVargRPC,
                               ::testing::Values(mv1, mv2));
