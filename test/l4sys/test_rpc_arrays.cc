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

/**
 * Harness for testing array parameters in RPC.
 *
 * The server interface defines functions `in_*` that take arrays as
 * input parameters and `out_*` functions with arrays as output parameters
 * which are populated with the content of `p_string`.
 *
 * Functions with input parameters also check that the received parameter is
 * still valid and return the result of this check to the client.
 */
struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  /// Error codes for failed checks (returned by functions with input parameters).
  enum Check_errors
  {
    Err_ok = 0,
    Err_start,   ///< String starts before the beginning of the UTCB area.
    Err_end,     ///< String ends after the end of the UTCB area.
    Err_not_ext, ///< An array that should have been copied still points to the UTCB.
  };

  long op_in_simple_str(Test_iface::Rights, L4::Ipc::Array_ref<char const> var)
  {
    p_string = std::string(var.data, var.length);

    // Check that the string is still on the UTCB.
    if ((l4_addr_t) var.data < (l4_addr_t) l4_utcb_mr())
      return Err_start;
    if ((l4_addr_t) var.data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return Err_end;

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
      return Err_not_ext;

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
      return Err_not_ext;

    return 0;
  }

  long op_in_opt_str(Test_iface::Rights, bool valid,
                     L4::Ipc::Array_ref<char const> var)
  {
    if (!valid)
      return 0;

    p_string = std::string(var.data, var.length);

    // Check that the string is still on the UTCB.
    if ((l4_addr_t) var.data < (l4_addr_t) l4_utcb_mr())
      return Err_start;
    if ((l4_addr_t) var.data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return Err_end;

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
    // Check that the string is still on the UTCB.
    if ((l4_addr_t) var->data < (l4_addr_t) l4_utcb_mr())
      return Err_start;
    if ((l4_addr_t) var->data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return Err_end;

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
  /**
   * Create an array type string parameter with arbitrary size and
   * send it to the server.
   *
   * The marshalling code already rejects overlong strings during sending.
   * To test if the receiving side can handle overlong strings correctly,
   * the message needs to be put together manually.
   */
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

/**
 * Strings are transferred with the correct length and content.
 */
TEST_F(ArrayTypesRPC, InSimpleString)
{
  char const *teststr = "Hello World";
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::Array<char const>(strlen(teststr),
                                                                teststr)));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  EXPECT_EQ(11U, handler().p_string.size());
}

/**
 * Empty strings are transferred correctly.
 */
TEST_F(ArrayTypesRPC, InEmptyString)
{
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::Array<char const>(0, 0)));
  EXPECT_EQ(0U, handler().p_string.length());
}

/**
 * String input that is longer than the capacity of the UTCB is rejected
 * when marshalling.
 */
TEST_F(ArrayTypesRPC, InOverlongString)
{
  char buf[L4_UTCB_GENERIC_DATA_SIZE * sizeof(l4_umword_t)];
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::Array<char const>(sizeof(buf) - 10,
                                                                buf)));
  EXPECT_EQ(-L4_EMSGTOOLONG,
            scap()->in_simple_str(L4::Ipc::Array<char const>(sizeof(buf), buf)));
}

/**
 * String input that is longer than the capacity of the UTCB is rejected
 * when unmarshalling on the server side.
 */
TEST_F(ArrayTypesRPC, InOverlongStringServerSide)
{
  l4_msgtag_t msg = manual_svrcpy_str(~0UL);
  ASSERT_EQ(-L4_EMSGTOOSHORT, l4_error(msg));
}

/**
 * String input that is longer than the message size as stated in the
 * message tag is rejected when unmarshalling on the server side.
 */
TEST_F(ArrayTypesRPC, InOverlongStringServerSideOverflowing)
{
  l4_msgtag_t msg = manual_svrcpy_str(1UL << (sizeof(unsigned long) * 8 - 1));
  ASSERT_EQ(-L4_EMSGTOOSHORT, l4_error(msg));
}

/**
 * Strings are copied out of the UTCB on server side when a buffer is
 * provided.
 */
TEST_F(ArrayTypesRPC, InSvrCpyString)
{
  char const *teststr = "The quick fox jumps.";
  L4::Ipc::Array<char const, unsigned long> a(strlen(teststr), teststr);
  EXPECT_EQ(0, scap()->in_svrcpy_str(a));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
}

/**
 * Strings are truncated on the server side if a buffer was provided that
 * was too short.
 */
TEST_F(ArrayTypesRPC, InSvrCpyServerBufferTooShort)
{
  char const *teststr = "XYfhe 4237f 64";
  L4::Ipc::Array<char const, unsigned long> a(strlen(teststr), teststr);
  EXPECT_EQ(0, scap()->in_svrcpy_smallbuf(a));
  EXPECT_STREQ("XY", handler().p_string.c_str());
}

/**
 * An empty string is not transmitted when the string parameter is optional.
 *
 * \note Currently does not test if the string really was not sent.
 */
TEST_F(ArrayTypesRPC, InOptStringInvalid)
{
  L4::Ipc::Opt<L4::Ipc::Array<char const> > in;
  EXPECT_EQ(0, scap()->in_opt_str(false, in));
}

/**
 * A string with content is transmitted when the string parameter is optional.
 */
TEST_F(ArrayTypesRPC, InOptStringValid)
{
  char const *teststr = "maybe";
  L4::Ipc::Array<char const> a(strlen(teststr), teststr);
  EXPECT_EQ(0, scap()->in_opt_str(true, a));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  EXPECT_EQ(5U, handler().p_string.size());
}

/**
 * Strings returned from the server are transferred correctly.
 */
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

/**
 * Returning the message fails if the string sent by the server
 * exceeds the capacity of the UTCB.
 */
TEST_F(ArrayTypesRPC, OutStringServerMessageTooLong)
{
  handler().p_string = std::string(4100, ' ');
  char buf[60];
  L4::Ipc::Array<char, unsigned long> ret(sizeof(buf), buf);
  EXPECT_EQ(-L4_EMSGTOOLONG, scap()->out_simple_str(ret));
}

/**
 * A string sent by the server is cut when the receive buffer of the
 * client is too short.
 */
TEST_F(ArrayTypesRPC, OutStringClientBufferTooShort)
{
  handler().p_string = std::string(100, ' ');
  char buf[60];
  L4::Ipc::Array<char, unsigned long> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_simple_str(ret));
  EXPECT_EQ(60U, ret.length);
}

/**
 * The client may read a returned string directly from the UTCB
 * when the parameter is a reference type.
 */
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

/**
 * When an array out parameter is a reference type, then the data returned
 * from the server is not copied even when a buffer is supplied.
 */
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

/**
 * The client may read a returned string directly from the UTCB
 * when the parameter is an optional reference array type.
 */
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

/**
 * Strings may be sent by the client and returned by the server in
 * the same call.
 */
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

/**
 * Arrays of arbitrary types are transferred correctly when sent by the client.
 */
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

/**
 * Arrays of arbitrary types are transferred correctly when sent by the server.
 */
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
