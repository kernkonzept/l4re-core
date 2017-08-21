/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test handling of different parameter types when marshalling and
 * unmarshalling RPC.
 */

#define CONFIG_ALLOW_REFS

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_iface>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Multi
{
  int x;
  int y;

  Multi(int px, int py) : x(px), y(py) {}
  Multi() : x(0), y(0) {}
};

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, by_value, (int));
  L4_INLINE_RPC(long, by_const_ref, (Multi const &));
  L4_INLINE_RPC(long, by_const_ptr, (Multi const *));
  L4_INLINE_RPC(long, by_ref, (Multi &));
  L4_INLINE_RPC(long, by_ptr, (Multi *));
  L4_INLINE_RPC(long, by_inout_ref, (L4::Ipc::In_out<Multi &>));
  L4_INLINE_RPC(long, by_inout_ptr, (L4::Ipc::In_out<Multi *>));
  L4_INLINE_RPC(long, by_opt_value, (L4::Ipc::Opt<int>));
  L4_INLINE_RPC(long, by_opt_value_complex, (L4::Ipc::Opt<Multi>));
  L4_INLINE_RPC(long, by_opt_ref, (bool, L4::Ipc::Opt<int &>));
  L4_INLINE_RPC(long, by_opt_const_ptr, (L4::Ipc::Opt<Multi const *>));
  L4_INLINE_RPC(long, by_opt_ptr, (bool, L4::Ipc::Opt<Multi *>));

  typedef L4::Typeid::Rpcs<by_value_t, by_const_ref_t, by_const_ptr_t,
                           by_ref_t, by_ptr_t, by_inout_ref_t, by_inout_ptr_t,
                           by_opt_value_t, by_opt_value_complex_t,
                           by_opt_ref_t, by_opt_const_ptr_t,
                           by_opt_ptr_t> Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_by_value(Test_iface::Rights, int val)
  {
    p_int = val;
    return 0;
  }

  long op_by_const_ref(Test_iface::Rights, Multi const &val)
  {
    p_multi = val;
    return 0;
  }

  long op_by_const_ptr(Test_iface::Rights, Multi const &val)
  {
    p_multi = val;
    return 0;
  }

  long op_by_ref(Test_iface::Rights, Multi &val)
  {
    val.x = 444;
    val.y = -444;
    return 0;
  }

  long op_by_ptr(Test_iface::Rights, Multi &val)
  {
    val.x = 888;
    val.y = -888;
    return 0;
  }

  long op_by_inout_ref(Test_iface::Rights, Multi &val)
  {
    p_multi = val;
    val.x = -1;
    val.y = 1;
    return 0;
  }

  long op_by_inout_ptr(Test_iface::Rights, Multi &val)
  {
    p_multi = val;
    val.x = -3;
    val.y = 3;
    return 0;
  }

  long op_by_opt_value(Test_iface::Rights, int val = 4)
  {
    p_int = val;
    return 0;
  }

  long op_by_opt_value_complex(Test_iface::Rights, Multi val)
  {
    p_multi = val;
    return 0;
  }

  long op_by_opt_ref(Test_iface::Rights, bool valid, L4::Ipc::Opt<int> &val)
  {
    EXPECT_FALSE(val.is_valid());
    val = -90;
    val.set_valid(valid);
    return 0;
  }

  long op_by_opt_ptr(Test_iface::Rights, bool valid, L4::Ipc::Opt<Multi> &val)
  {
    EXPECT_FALSE(val.is_valid());
    val->x = 42;
    val->y = 50;
    val.set_valid(valid);
    return 0;
  }

  long op_by_opt_const_ptr(Test_iface::Rights, Multi const &val)
  {
    p_multi = val;
    return 0;
  }

  int p_int;
  Multi p_multi;
};

typedef Atkins::Fixture::Epiface_thread<Test_handler> ParamTypesRPC;

/**
 * Value parameters are transferred as input parameters.
 */
TEST_F(ParamTypesRPC, ByValue)
{
  ASSERT_EQ(0, scap()->by_value(34));
  EXPECT_EQ(34, handler().p_int);
}

/**
 * Const reference parameters are transferred as input parameters.
 */
TEST_F(ParamTypesRPC, ByConstRef)
{
  ASSERT_EQ(0, scap()->by_const_ref(Multi(9, 10)));
  EXPECT_EQ(9, handler().p_multi.x);
  EXPECT_EQ(10, handler().p_multi.y);
}

/**
 * Const pointer parameters are transferred as input parameters.
 */
TEST_F(ParamTypesRPC, ByConstPtr)
{
  Multi m(22, 33);
  ASSERT_EQ(0, scap()->by_const_ptr(&m));
  EXPECT_EQ(22, handler().p_multi.x);
  EXPECT_EQ(33, handler().p_multi.y);
}

/**
 * Non-const reference parameters are transferred as output parameters.
 */
TEST_F(ParamTypesRPC, ByOutRef)
{
  Multi m(1234567, 7654321);
  ASSERT_EQ(0, scap()->by_ref(m));
  EXPECT_EQ(444, m.x);
  EXPECT_EQ(-444, m.y);
}

/**
 * Non-const pointer parameters are transferred as output parameters.
 */
TEST_F(ParamTypesRPC, ByOutPtr)
{
  Multi m(12567, 76521);
  ASSERT_EQ(0, scap()->by_ptr(&m));
  EXPECT_EQ(888, m.x);
  EXPECT_EQ(-888, m.y);
}

/**
 * Reference parameters marked as input/output are transferred correctly.
 */
TEST_F(ParamTypesRPC, ByInoutRef)
{
  Multi m(555, 901);
  ASSERT_EQ(0, scap()->by_inout_ref(m));
  EXPECT_EQ(555, handler().p_multi.x);
  EXPECT_EQ(901, handler().p_multi.y);
  EXPECT_EQ(-1, m.x);
  EXPECT_EQ(1, m.y);
}

/**
 * Pointer parameters marked as input/output are transferred correctly.
 */
TEST_F(ParamTypesRPC, ByInoutPtr)
{
  Multi m(55, 123);
  ASSERT_EQ(0, scap()->by_inout_ptr(&m));
  EXPECT_EQ(55, handler().p_multi.x);
  EXPECT_EQ(123, handler().p_multi.y);
  EXPECT_EQ(-3, m.x);
  EXPECT_EQ(3, m.y);
}

/**
 * Optional values with valid content are transferred correctly
 * as input parameters.
 */
TEST_F(ParamTypesRPC, ByOptValue)
{
  ASSERT_EQ(0, scap()->by_opt_value(835));
  EXPECT_EQ(835, handler().p_int);
  L4::Ipc::Opt<int> val;
  ASSERT_FALSE(val.is_valid());
  ASSERT_EQ(0, scap()->by_opt_value(val));
}

/**
 * Optional structs with valid content are transferred correctly
 * as input parameters.
 */
TEST_F(ParamTypesRPC, ByOptValyueComplex)
{
  L4::Ipc::Opt<Multi> val;
  val = Multi(5,5);
  ASSERT_EQ(0, scap()->by_opt_value_complex(val));
  EXPECT_EQ(5, handler().p_multi.x);
  EXPECT_EQ(5, handler().p_multi.y);
  val.set_valid(false);
  ASSERT_EQ(0, scap()->by_opt_value_complex(val));
  EXPECT_EQ(0, handler().p_multi.x);
  EXPECT_EQ(0, handler().p_multi.y);
}

/**
 * Optional reference parameters are output parameters and transferred correctly.
 */
TEST_F(ParamTypesRPC, ByOptRef)
{
  int ret;
  ASSERT_EQ(0, scap()->by_opt_ref(true, ret));
  EXPECT_EQ(-90, ret);
  ret = 3537;
  ASSERT_EQ(0, scap()->by_opt_ref(false, ret));
  EXPECT_EQ(3537, ret);
}

/**
 * Optional pointer parameters are output parameters and transferred correctly.
 */
TEST_F(ParamTypesRPC, ByOptPtr)
{
  Multi m(11, 22);
  ASSERT_EQ(0, scap()->by_opt_ptr(true, &m));
  EXPECT_EQ(42, m.x);
  EXPECT_EQ(50, m.y);
  m = Multi(-99999, -99999);
  ASSERT_EQ(0, scap()->by_opt_ptr(false, &m));
  EXPECT_EQ(-99999, m.x);
  EXPECT_EQ(-99999, m.y);
}

/**
 * Optional constant pointer parameters are input parameters and transferred
 * correctly.
 */
TEST_F(ParamTypesRPC, ByOptConstPtr)
{
  Multi m(9, 10);
  ASSERT_EQ(0, scap()->by_opt_const_ptr(&m));
  EXPECT_EQ(9, handler().p_multi.x);
  EXPECT_EQ(10, handler().p_multi.y);
  L4::Ipc::Opt<Multi const *> val;
  EXPECT_FALSE(val.is_valid());
  ASSERT_EQ(0, scap()->by_opt_const_ptr(val));
  EXPECT_EQ(0, handler().p_multi.x);
  EXPECT_EQ(0, handler().p_multi.y);
  m = Multi(-99999, -99999);
  val = &m;
  ASSERT_EQ(0, scap()->by_opt_const_ptr(val));
  EXPECT_EQ(-99999, handler().p_multi.x);
  EXPECT_EQ(-99999, handler().p_multi.y);
}
