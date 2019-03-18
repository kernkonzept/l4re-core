/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test basic marshalling and unmarshalling functionality of RPC.
 */

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_iface>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

#include <climits>

struct Test_iface : L4::Kobject_t<Test_iface, L4::Kobject>
{
  L4_INLINE_RPC(long, null, ());
  L4_INLINE_RPC(l4_msgtag_t, null_tag, ());
  L4_INLINE_RPC(long, send_only, (), L4::Ipc::Send_only);
  L4_INLINE_RPC(l4_msgtag_t, send_only_tag, (), L4::Ipc::Send_only);
  L4_INLINE_RPC(long, put_int, (int, int *));
  L4_INLINE_RPC(long, put_uint, (unsigned int, unsigned int *));
  L4_INLINE_RPC(long, put_char, (signed char, signed char *));
  L4_INLINE_RPC(long, put_uchar, (unsigned char, unsigned char *));
  L4_INLINE_RPC(long, put_uint64, (l4_uint64_t, l4_uint64_t*));
  typedef L4::Typeid::Rpcs<null_t, null_tag_t, send_only_t, send_only_tag_t,
                           put_int_t, put_uint_t,
                           put_char_t, put_uchar_t, put_uint64_t> Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_null(Test_iface::Rights)
  { return retval; }

  long op_null_tag(Test_iface::Rights)
  { return retval; }

  long op_send_only(Test_iface::Rights)
  { return -L4_ENOREPLY; }

  long op_send_only_tag(Test_iface::Rights)
  { return -L4_ENOREPLY; }

  long op_put_int(Test_iface::Rights, int in, int &out)
  {
    p_int = in;
    out = in;
    return 2;
  }

  long op_put_uint(Test_iface::Rights, unsigned int in, unsigned int &out)
  {
    p_uint = in;
    out = in;
    return 3;
  }

  long op_put_char(Test_iface::Rights, signed char in, signed char &out)
  {
    p_char = in;
    out = in;
    return 4;
  }

  long op_put_uchar(Test_iface::Rights, unsigned char in, unsigned char &out)
  {
    p_uchar = in;
    out = in;
    return 5;
  }

  long op_put_uint64(Test_iface::Rights, l4_uint64_t in, l4_uint64_t &out)
  {
    p_uint64 = in;
    out = in;
    return 6;
  }

  int p_int;
  unsigned int p_uint;
  signed char p_char;
  unsigned char p_uchar;
  l4_uint64_t p_uint64;
  long retval;
};

struct SimpleTypesRPC : Atkins::Fixture::Epiface_thread<Test_handler>
{
  void expect_retval(long val)
  {
    handler().retval = val;
    EXPECT_EQ(val, scap()->null());
    EXPECT_EQ(val, scap()->null_tag().label());
  }
};

/**
 * Return values are transferred correctly.
 */
TEST_F(SimpleTypesRPC, ReturnValues)
{
  expect_retval(0);
  expect_retval(1);
  expect_retval(-23);
  expect_retval(-L4_EINVAL);
  expect_retval(LONG_MIN >> 16);
  expect_retval(LONG_MAX >> 16);
  expect_retval(0);
}

/**
 * Send-only IPC is transferred correctly.
 */
TEST_F(SimpleTypesRPC, SendOnly)
{
  EXPECT_EQ(0, scap()->send_only());
  EXPECT_EQ(0UL, l4_ipc_error(scap()->send_only_tag(), l4_utcb()));
}

/**
 * Integer parameters are transferred correctly to and from the server.
 */
TEST_F(SimpleTypesRPC, IntValues)
{
  int ret;

  EXPECT_EQ(2, scap()->put_int(0, &ret));
  EXPECT_EQ(0, ret);
  EXPECT_EQ(0, handler().p_int);

  EXPECT_EQ(2, scap()->put_int(101, &ret));
  EXPECT_EQ(101, ret);
  EXPECT_EQ(101, handler().p_int);

  EXPECT_EQ(2, scap()->put_int(-5, &ret));
  EXPECT_EQ(-5, ret);
  EXPECT_EQ(-5, handler().p_int);

  EXPECT_EQ(2, scap()->put_int(INT_MAX, &ret));
  EXPECT_EQ(INT_MAX, ret);
  EXPECT_EQ(INT_MAX, handler().p_int);

  EXPECT_EQ(2, scap()->put_int(INT_MIN, &ret));
  EXPECT_EQ(INT_MIN, ret);
  EXPECT_EQ(INT_MIN, handler().p_int);

  EXPECT_EQ(2, scap()->put_int(0, &ret));
  EXPECT_EQ(0, ret);
  EXPECT_EQ(0, handler().p_int);
}

/**
 * Unsigned integer parameters are transferred correctly to and from the server.
 */
TEST_F(SimpleTypesRPC, UIntValues)
{
  unsigned int ret;

  EXPECT_EQ(3, scap()->put_uint(0, &ret));
  EXPECT_EQ(0U, ret);
  EXPECT_EQ(0U, handler().p_uint);

  EXPECT_EQ(3, scap()->put_uint(101, &ret));
  EXPECT_EQ(101U, ret);
  EXPECT_EQ(101U, handler().p_uint);

  EXPECT_EQ(3, scap()->put_uint(UINT_MAX - 1, &ret));
  EXPECT_EQ(UINT_MAX - 1, ret);
  EXPECT_EQ(UINT_MAX - 1, handler().p_uint);

  EXPECT_EQ(3, scap()->put_uint(UINT_MAX, &ret));
  EXPECT_EQ(UINT_MAX, ret);
  EXPECT_EQ(UINT_MAX, handler().p_uint);

  EXPECT_EQ(3, scap()->put_uint(0, &ret));
  EXPECT_EQ(0U, ret);
  EXPECT_EQ(0U, handler().p_uint);
}

/**
 * Char parameters are transferred correctly to and from the server.
 */
TEST_F(SimpleTypesRPC, CharValues)
{
  signed char ret;

  EXPECT_EQ(4, scap()->put_char(0, &ret));
  EXPECT_EQ(0, ret);
  EXPECT_EQ(0, handler().p_char);

  EXPECT_EQ(4, scap()->put_char(101, &ret));
  EXPECT_EQ(101, ret);
  EXPECT_EQ(101, handler().p_char);

  EXPECT_EQ(4, scap()->put_char(-5, &ret));
  EXPECT_EQ(-5, ret);
  EXPECT_EQ(-5, handler().p_char);

  EXPECT_EQ(4, scap()->put_char(SCHAR_MAX, &ret));
  EXPECT_EQ(SCHAR_MAX, ret);
  EXPECT_EQ(SCHAR_MAX, handler().p_char);

  EXPECT_EQ(4, scap()->put_char(SCHAR_MIN, &ret));
  EXPECT_EQ(SCHAR_MIN, ret);
  EXPECT_EQ(SCHAR_MIN, handler().p_char);

  EXPECT_EQ(4, scap()->put_char(0, &ret));
  EXPECT_EQ(0, ret);
  EXPECT_EQ(0, handler().p_char);
}

/**
 * Unsigned char parameters are transferred correctly to and from the server.
 */
TEST_F(SimpleTypesRPC, UCharValues)
{
  unsigned char ret;

  EXPECT_EQ(5, scap()->put_uchar(0, &ret));
  EXPECT_EQ(0U, ret);
  EXPECT_EQ(0U, handler().p_uchar);

  EXPECT_EQ(5, scap()->put_uchar(101, &ret));
  EXPECT_EQ(101U, ret);
  EXPECT_EQ(101U, handler().p_uchar);

  EXPECT_EQ(5, scap()->put_uchar(UCHAR_MAX - 1, &ret));
  EXPECT_EQ(UCHAR_MAX - 1, ret);
  EXPECT_EQ(UCHAR_MAX - 1, handler().p_uchar);

  EXPECT_EQ(5, scap()->put_uchar(UCHAR_MAX, &ret));
  EXPECT_EQ(UCHAR_MAX, ret);
  EXPECT_EQ(UCHAR_MAX, handler().p_uchar);

  EXPECT_EQ(5, scap()->put_uchar(0, &ret));
  EXPECT_EQ(0U, ret);
  EXPECT_EQ(0U, handler().p_uchar);
}

/**
 * Unsigned 64bit integers are transferred correctly to and from the server.
 */
TEST_F(SimpleTypesRPC, UInt64Values)
{
  l4_uint64_t ret;

  EXPECT_EQ(6, scap()->put_uint64(0, &ret));
  EXPECT_EQ(0U, ret);
  EXPECT_EQ(0U, handler().p_uint64);

  EXPECT_EQ(6, scap()->put_uint64(101, &ret));
  EXPECT_EQ(101U, ret);
  EXPECT_EQ(101U, handler().p_uint64);

  EXPECT_EQ(6, scap()->put_uint64(1ULL << 63, &ret));
  EXPECT_EQ(1ULL << 63, ret);
  EXPECT_EQ(1ULL << 63, handler().p_uint64);

  EXPECT_EQ(6, scap()->put_uint64(~1ULL, &ret));
  EXPECT_EQ(~1ULL, ret);
  EXPECT_EQ(~1ULL, handler().p_uint64);

  EXPECT_EQ(6, scap()->put_uint64(0, &ret));
  EXPECT_EQ(0U, ret);
  EXPECT_EQ(0U, handler().p_uint64);
}

/**
 * Calls using a capability that is not present fail.
 */
TEST_F(SimpleTypesRPC, InvalidCapCall)
{
  ASSERT_EQ(L4_EOK, l4_error(L4Re::Env::env()->task()->delete_obj(scap())));

  EXPECT_EQ(-L4_EIPC_LO - L4_IPC_ENOT_EXISTENT, scap()->null());
  EXPECT_EQ(L4_IPC_ENOT_EXISTENT,
            l4_ipc_error(scap()->null_tag(), l4_utcb()));

  EXPECT_EQ(-L4_EIPC_LO - L4_IPC_ENOT_EXISTENT, scap()->send_only());
  EXPECT_EQ(L4_IPC_ENOT_EXISTENT,
            l4_ipc_error(scap()->send_only_tag(), l4_utcb()));
}
