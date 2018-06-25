/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test marshalling and unmarshalling of arrays in RPC.
 */

#include <cstdlib>

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_iface>
#include <l4/sys/cxx/ipc_array>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Tuple
{
  l4_uint32_t first;
  l4_uint8_t second;

  // Note: we don't care about true randomness here.
  // Just want to fill the tuple with noise.
  Tuple() : first(rand() & 0xFFFFFFFF), second(rand() & 0xFF) {}

  Tuple(l4_uint8_t s) : first(rand() & 0xFFFFFFFF), second(s) {}

  Tuple(l4_uint32_t f, l4_uint8_t s) : first(f), second(s) {}
};

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, in_simple_str, (L4::Ipc::Array<char const>));
  L4_INLINE_RPC(long, in_svrcpy_str,
                (L4::Ipc::Array<char const, unsigned long>));
  L4_INLINE_RPC(long, in_svrcpy_smallbuf,
                (L4::Ipc::Array<char const, unsigned long>));
  L4_INLINE_RPC(long, in_opt_str,
                (bool, L4::Ipc::Opt<L4::Ipc::Array<char const> >));

  L4_INLINE_RPC(long, out_simple_str, (L4::Ipc::Array<char, unsigned long> &));
  L4_INLINE_RPC(long, out_inutcb_str,
                (L4::Ipc::Array_ref<char, unsigned long> &));
  L4_INLINE_RPC(long, out_inutcb_opt,
                (bool, L4::Ipc::Opt<L4::Ipc::Array_ref<char const> &>));

  L4_INLINE_RPC(long, inout_str,
                (L4::Ipc::Array<char const>, L4::Ipc::Array<char> &));


  L4_INLINE_RPC(long, in_struct, (L4::Ipc::Array<Tuple const>));
  L4_INLINE_RPC(long, out_struct, (L4::Ipc::Array<Tuple> &));


  typedef L4::Typeid::Rpcs<in_simple_str_t, in_svrcpy_str_t,
                           in_svrcpy_smallbuf_t, in_opt_str_t, out_simple_str_t,
                           out_inutcb_str_t, out_inutcb_opt_t, inout_str_t,
                           in_struct_t, out_struct_t
                          > Rpcs;
};

struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  long op_in_simple_str(Test_iface::Rights, L4::Ipc::Array_ref<char const> var)
  {
    p_string = std::string(var.data, var.length);

    // Check that the string is still on the UTB.
    if ((l4_addr_t) var.data < (l4_addr_t) l4_utcb_mr())
      return 1;
    if ((l4_addr_t) var.data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return 2;

    return 0;
  }

  long op_in_svrcpy_str(Test_iface::Rights,
                        L4::Ipc::Array_in_buf<char, unsigned long> const &var)
  {
    p_string = std::string(var.data, var.length);

    // Check that the string is outside the UTCB.
    l4_addr_t daddr = (l4_addr_t) var.data;
    if (daddr >= (l4_addr_t) l4_utcb_mr()
        && daddr <= (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return 1;

    return 0;
  }

  long op_in_svrcpy_smallbuf(Test_iface::Rights,
                             L4::Ipc::Array_in_buf<char, unsigned long, 2> const &var)
  {
    p_string = std::string(var.data, var.length);

    // Check that the string is outside the UTCB.
    l4_addr_t daddr = (l4_addr_t) var.data;
    if (daddr >= (l4_addr_t) l4_utcb_mr()
        && daddr <= (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return 1;

    return 0;
  }

  long op_in_opt_str(Test_iface::Rights, bool valid,
                     L4::Ipc::Array_ref<char const> var)
  {
    if (!valid)
      return 0;

    p_string = std::string(var.data, var.length);

    // Check that the string is still on the UTB.
    if ((l4_addr_t) var.data < (l4_addr_t) l4_utcb_mr())
      return 1;
    if ((l4_addr_t) var.data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return 2;

    return 0;
  }

  long op_out_simple_str(Test_iface::Rights,
                         L4::Ipc::Array_ref<char, unsigned long> &var)
  {
    p_size = var.length;
    if (var.length >= p_string.length())
      memcpy(var.data, p_string.data(), p_string.length());
    var.length = p_string.length();

    return 0;
  }

  long op_out_inutcb_str(Test_iface::Rights,
                         L4::Ipc::Array_ref<char, unsigned long> &var)
  {
    memcpy(var.data, p_string.data(), p_string.length());
    var.length = p_string.length();

    return 0;
  }

  long op_out_inutcb_opt(Test_iface::Rights, bool valid,
                         L4::Ipc::Opt<L4::Ipc::Array_ref<char>> &var)
  {
    // Check that the string is still on the UTB.
    if ((l4_addr_t) var->data < (l4_addr_t) l4_utcb_mr())
      return 1;
    if ((l4_addr_t) var->data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return 2;

    if (valid)
      {
        memcpy(var->data, p_string.data(), p_string.length());
        var->length = p_string.length();
      }
    var.set_valid(valid);

    return 0;
  }

  long op_inout_str(Test_iface::Rights, L4::Ipc::Array_ref<char const> instr,
                    L4::Ipc::Array_ref<char> &outstr)
  {
    std::string tmp = std::string(instr.data, instr.length);

    memcpy(outstr.data, p_string.data(), p_string.length());
    outstr.length = p_string.length();

    p_string = tmp;

    return 0;
  }

  long op_in_struct(Test_iface::Rights,
                    L4::Ipc::Array_ref<Tuple const> var)
  {
    p_vec.clear();

    for (unsigned i = 0; i < var.length; ++i)
      p_vec.push_back(var.data[i]);

    return 0;
  }

  long op_out_struct(Test_iface::Rights,
                     L4::Ipc::Array_ref<Tuple> &var)
  {
    p_size = var.length;

    if (var.length < p_vec.size())
      return -L4_EINVAL;

    for (unsigned i = 0; i < p_vec.size(); ++i)
      {
        var.data[i].first = p_vec[i].first;
        var.data[i].second = p_vec[i].second;
      }

    var.length = p_vec.size();

    return 0;
  }

  std::string p_string;
  unsigned p_size;
  std::vector<Tuple> p_vec;
};

struct ArrayTypesRPC : Atkins::Fixture::Epiface_thread<Test_handler>
{
  l4_msgtag_t manual_svrcpy_str(unsigned long arraysz)
  {
    // Hand-craft a message with an oversized string.
    typedef Test_iface::Rpcs::opcode_type opcode_type;
    using L4::Ipc::Msg::align_to;
    l4_addr_t mraddr = reinterpret_cast<l4_addr_t>(l4_utcb_mr()->mr);
    l4_addr_t offset = align_to<opcode_type>(0);
    *(reinterpret_cast<opcode_type *>(mraddr + offset)) =
      Test_iface::Rpcs::Rpc<Test_iface::in_svrcpy_str_t>::Opcode;
    offset = align_to<unsigned long>(offset + sizeof(opcode_type));
    *(reinterpret_cast<unsigned long *>(mraddr + offset)) = arraysz;
    offset = align_to<l4_umword_t>(offset + sizeof(unsigned long));

    return l4_ipc_call(scap().cap(), l4_utcb(),
                       l4_msgtag(Test_iface::Protocol,
                                 offset / sizeof(l4_umword_t), 0, 0),
                       L4_IPC_NEVER);
  }
};

TEST_F(ArrayTypesRPC, InSimpleString)
{
  char const *teststr = "Hello World";
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::Array<char const>(strlen(teststr),
                                                                teststr)));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  EXPECT_EQ(11U, handler().p_string.size());
}

TEST_F(ArrayTypesRPC, InEmptyString)
{
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::Array<char const>(0, 0)));
  EXPECT_EQ(0U, handler().p_string.length());
}

TEST_F(ArrayTypesRPC, InOverlongString)
{
  char buf[L4_UTCB_GENERIC_DATA_SIZE * sizeof(l4_umword_t)];
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::Array<char const>(sizeof(buf) - 10,
                                                                buf)));
  EXPECT_EQ(-L4_EMSGTOOLONG,
            scap()->in_simple_str(L4::Ipc::Array<char const>(sizeof(buf), buf)));
}

TEST_F(ArrayTypesRPC, InOverlongStringServerSide)
{
  l4_msgtag_t msg = manual_svrcpy_str(~0UL);
  ASSERT_EQ(-L4_EMSGTOOSHORT, l4_error(msg));
}

TEST_F(ArrayTypesRPC, InOverlongStringServerSideOverflowing)
{
  // Test for overflow in message size computation.
  l4_msgtag_t msg = manual_svrcpy_str(1UL << (sizeof(unsigned long) * 8 - 1));
  ASSERT_EQ(-L4_EMSGTOOSHORT, l4_error(msg));
}

TEST_F(ArrayTypesRPC, InSvrCpyString)
{
  char const *teststr = "The quick fox jumps.";
  L4::Ipc::Array<char const, unsigned long> a(strlen(teststr), teststr);
  EXPECT_EQ(0, scap()->in_svrcpy_str(a));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
}

TEST_F(ArrayTypesRPC, InSvrCpyServerBufferTooShort)
{
  char const *teststr = "XYfhe 4237f 64";
  L4::Ipc::Array<char const, unsigned long> a(strlen(teststr), teststr);
  EXPECT_EQ(0, scap()->in_svrcpy_smallbuf(a));
  // message is simply cut
  EXPECT_STREQ("XY", handler().p_string.c_str());
}

TEST_F(ArrayTypesRPC, InOptStringInvalid)
{
  L4::Ipc::Opt<L4::Ipc::Array<char const> > in;
  EXPECT_EQ(0, scap()->in_opt_str(false, in));
}

TEST_F(ArrayTypesRPC, InOptStringValid)
{
  char const *teststr = "maybe";
  L4::Ipc::Array<char const> a(strlen(teststr), teststr);
  EXPECT_EQ(0, scap()->in_opt_str(true, a));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  EXPECT_EQ(5U, handler().p_string.size());
}

TEST_F(ArrayTypesRPC, OutSimpleString)
{
  handler().p_string = "Goodbye Friends.";
  char buf[60];
  L4::Ipc::Array<char, unsigned long> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_simple_str(ret));
  ASSERT_EQ(16U, ret.length);
  EXPECT_EQ(handler().p_string, std::string(ret.data, ret.length));
  EXPECT_EQ(handler().p_string, std::string(buf, ret.length));
}

TEST_F(ArrayTypesRPC, OutStringServerMessageTooLong)
{
  handler().p_string = std::string(4100, ' ');
  char buf[60];
  L4::Ipc::Array<char, unsigned long> ret(sizeof(buf), buf);
  EXPECT_EQ(-L4_EMSGTOOLONG, scap()->out_simple_str(ret));
}

TEST_F(ArrayTypesRPC, OutStringClientBufferTooShort)
{
  handler().p_string = std::string(100, ' ');
  char buf[60];
  L4::Ipc::Array<char, unsigned long> ret(sizeof(buf), buf);
  // message is simply cut
  EXPECT_EQ(0, scap()->out_simple_str(ret));
  EXPECT_EQ(60U, ret.length);
}

TEST_F(ArrayTypesRPC, OutInUtcbString)
{
  handler().p_string = "Westward ho.";
  L4::Ipc::Array_ref<char, unsigned long> ret;
  EXPECT_EQ(0, scap()->out_inutcb_str(ret));
  ASSERT_EQ(12U, ret.length);
  EXPECT_EQ(handler().p_string, std::string(ret.data, ret.length));
  EXPECT_GT((l4_addr_t) ret.data, (l4_addr_t) l4_utcb_mr());
  EXPECT_LT((l4_addr_t) ret.data,
            (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t));
}

TEST_F(ArrayTypesRPC, OutInUtcbStringArray)
{
  handler().p_string = "Westward ho.";
  char buf[60] = "x";
  L4::Ipc::Array<char, unsigned long> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_inutcb_str(ret));
  ASSERT_EQ(12U, ret.length);
  EXPECT_EQ(handler().p_string, std::string(ret.data, ret.length));
  EXPECT_STREQ("x", buf);
  EXPECT_GT((l4_addr_t) ret.data, (l4_addr_t) l4_utcb_mr());
  EXPECT_LT((l4_addr_t) ret.data,
            (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t));
}

TEST_F(ArrayTypesRPC, OutInUtcbStringOptArray)
{
  handler().p_string = "Westward ho.";
  char buf[60] = "x";
  L4::Ipc::Array<char const> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_inutcb_opt(true, ret));
  ASSERT_EQ(12U, ret.length);
  EXPECT_EQ(handler().p_string, std::string(ret.data, ret.length));
  EXPECT_STREQ("x", buf);
  EXPECT_GT((l4_addr_t) ret.data, (l4_addr_t) l4_utcb_mr());
  EXPECT_LT((l4_addr_t) ret.data,
            (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t));
}

TEST_F(ArrayTypesRPC, InOutString)
{
  char const *teststr = "something short";
  char const *testout = "something a bit longer";
  handler().p_string = testout;
  char buf[100];
  L4::Ipc::Array<char> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->inout_str(L4::Ipc::Array<char const>(strlen(teststr),
                                                            teststr),
                                 ret));
  EXPECT_EQ(15U, handler().p_string.size());
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  ASSERT_EQ(22U, ret.length);
  EXPECT_EQ(std::string(testout, strlen(testout)),
            std::string(ret.data, ret.length));
}

TEST_F(ArrayTypesRPC, InStruct)
{
  Tuple buf[10]; // randomly filled
  EXPECT_EQ(0, scap()->in_struct(L4::Ipc::Array<Tuple const>(10, buf)));
  ASSERT_EQ(10U, handler().p_vec.size());

  for (int i = 0; i < 10; ++i)
    {
      EXPECT_EQ(buf[i].first, handler().p_vec[i].first);
      EXPECT_EQ(buf[i].second, handler().p_vec[i].second);
    }

}

TEST_F(ArrayTypesRPC, OutStruct)
{
  handler().p_vec.clear();
  for (int i = 0; i < 6; ++i)
    handler().p_vec.push_back(Tuple()); // randomly filled

  Tuple buf[20];
  L4::Ipc::Array<Tuple> ret(20, buf);
  EXPECT_EQ(0, scap()->out_struct(ret));
  ASSERT_EQ(handler().p_vec.size(), ret.length);
  ASSERT_EQ(buf, ret.data);

  for (int i = 0; i < ret.length; ++i)
    {
      EXPECT_EQ(handler().p_vec[i].first, buf[i].first);
      EXPECT_EQ(handler().p_vec[i].second, buf[i].second);
    }

}
