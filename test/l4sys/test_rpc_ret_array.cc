/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test handling of capability ret_array parameter when marshalling and
 * unmarshalling RPC.
 */

#include <l4/sys/cxx/ipc_iface>
#include <l4/sys/cxx/ipc_ret_array>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Foo
{
  char c;
  int i;

  Foo(char _c, int _i) : c(_c), i(_i) {}

  bool operator==(Foo const &o) const
  {return o.c == c && o.i == i; }
};

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, get_int, (L4::Ipc::Ret_array<int>));
  L4_INLINE_RPC(long, get_foo, (L4::Ipc::Ret_array<Foo>));
  L4_INLINE_RPC(long, set_ret, (long, L4::Ipc::Ret_array<int>));

  typedef L4::Typeid::Rpcs<get_int_t, get_foo_t, set_ret_t> Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_get_int(Test_iface::Rights, L4::Ipc::Ret_array<int> &a)
  { return handle(p_int, a); }

  long op_get_foo(Test_iface::Rights, L4::Ipc::Ret_array<Foo> &a)
  { return handle(p_foo, a); }

  long op_set_ret(Test_iface::Rights, long r, L4::Ipc::Ret_array<int> &a)
  {
    a.value[0] = 1234;
    return r;
  }

  template <typename T>
  long handle(std::vector<T> &in, L4::Ipc::Ret_array<T> &out)
  {
    unsigned num_elem = in.size();

    if (num_elem > out.max)
      num_elem = out.max;

    for (unsigned char i = 0; i < num_elem; ++i)
      out.value[i] = in[i];

    return num_elem;
  }

  std::vector<int> p_int;
  std::vector<Foo> p_foo;
};

struct RetArrayRPC : Atkins::Fixture::Epiface_thread<Test_handler>
{
  template <typename T>
  void compare_result(T const *out, std::vector<T> const &exp)
  {
    for (size_t i = 0; i < exp.size(); ++i)
      EXPECT_EQ(out[i], exp[i]);
  }
};

/**
 * Return arrays with simple integers are correctly transferred.
 */
TEST_F(RetArrayRPC, SimpleIntArray)
{
  auto &iarr = handler().p_int;
  for (unsigned i = 0; i < 23; ++i)
    iarr.push_back(rand() & ~0U);
  int const *ret = 0;
  ASSERT_EQ(long(iarr.size()), scap()->get_int(&ret));
  compare_result(ret, iarr);
}

/**
 * An empty return array is correctly transferred.
 */
TEST_F(RetArrayRPC, EmptyIntArray)
{
  int const *ret = 0;
  ASSERT_EQ(0L, scap()->get_int(&ret));
}

/**
 * Return arrays with complex structs are correctly transferred.
 */
TEST_F(RetArrayRPC, SimpleFooArray)
{
  auto &iarr = handler().p_foo;
  for (unsigned i = 0; i < 31; ++i)
    iarr.push_back(Foo(rand() & 0xFF, rand() & ~0U));
  Foo const *ret = 0;
  ASSERT_EQ(long(iarr.size()), scap()->get_foo(&ret));
  compare_result(ret, iarr);
}

/**
 * If the server tries to return an array that does not fit,
 * an error is returned.
 */
TEST_F(RetArrayRPC, TooLargeReturnArray)
{
  int const *ret = 0;
  EXPECT_EQ(-L4_EMSGTOOLONG, scap()->set_ret(10000, &ret));
}

/**
 * Custom errors set by the server are mashalled correctly.
 */
TEST_F(RetArrayRPC, ReturnError)
{
  int const *ret = 0;
  ASSERT_EQ(-L4_EINVAL, scap()->set_ret(-L4_EINVAL, &ret));
}
