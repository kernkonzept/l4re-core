/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test marshalling and unmarshalling of null-terminated strings in RPC.
 */

#include <cstdlib>
#include <cstring>

#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_iface>
#include <l4/sys/cxx/ipc_string>
#include <l4/sys/kdebug.h>

#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/tap/main>

struct Test_iface : L4::Kobject_0t<Test_iface>
{
  L4_INLINE_RPC(long, in_simple_str, (L4::Ipc::String<>));
  L4_INLINE_RPC(long, out_simple_str, (L4::Ipc::String<char> &));
  L4_INLINE_RPC(long, in_opt_str, (bool, L4::Ipc::Opt<L4::Ipc::String<>>));
  L4_INLINE_RPC(long, out_opt_str, (bool, L4::Ipc::Opt<L4::Ipc::String<char> &>));

  typedef L4::Typeid::Rpcs<in_simple_str_t, out_simple_str_t,
                           in_opt_str_t, out_opt_str_t
                           > Rpcs;
};

/**
 * Harness for testing string parameters in RPC.
 *
 * Functions with string input parameters check that the received string
 * is valid and store the result in `p_string` for the test functions to
 * inspect. Functions with string output parameters send the string stored
 * in `p_string` back to the caller.
 */
struct Test_handler : L4::Epiface_t<Test_handler, Test_iface>
{
  /// Error codes for failed checks (returned by functions with input parameters).
  enum Check_errors
  {
    Err_ok = 0,
    Err_start,  ///< String starts before the beginning of the UTCB area.
    Err_end,    ///< String ends after the end of the UTCB area.
    Err_term,   ///< String not null-terminated.
    Err_size,   ///< String length is zero or larger than 2^16.
    Err_null,   ///< String pointer invalid
  };

  long op_in_simple_str(Test_iface::Rights, L4::Ipc::String<> var)
  {
    // Check that the string is still in the UTCB.
    if ((l4_addr_t) var.data < (l4_addr_t) l4_utcb_mr())
      return Err_start;
    if ((l4_addr_t) var.data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return Err_end;

    return save_str(var);
  }

  long op_out_simple_str(Test_iface::Rights, L4::Ipc::String<char> &var)
  {
    p_size = var.length;
    if (var.length >= p_string.length())
      memcpy(var.data, p_string.data(), p_string.length());
    var.length = p_string.length();

    return 0;
  }

  long op_in_opt_str(Test_iface::Rights, bool valid, L4::Ipc::String<> var)
  {
    if (valid)
      return save_str(var);
    else
      return 0;
  }

  long op_out_opt_str(Test_iface::Rights, bool valid,
                      L4::Ipc::Opt<L4::Ipc::String<char> > &var)
  {
    // Check that the string is still in the UTCB.
    if ((l4_addr_t) var->data < (l4_addr_t) l4_utcb_mr())
      return Err_start;
    if ((l4_addr_t) var->data > (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t))
      return Err_end;

    p_size = var->length;
    if (valid)
      {
        if (var->length >= p_string.length())
          memcpy(var->data, p_string.data(), p_string.length());
        var->length = p_string.length();
      }
    var.set_valid(valid);

    return 0;
  }

private:
  long save_str(L4::Ipc::String<> const &var)
  {
    p_size = var.length;

    // String must have at least a size of 1 (for the final \0)
    if (p_size == 0 || p_size > 0xFFFF)
      return Err_size;

    if (!var.data)
      return Err_null;

    // Check that the string is null-terminated
    if (var.data[p_size - 1] != '\0')
      return Err_term;

    p_string = std::string(var.data, var.length - 1);

    return 0;
  }

public:
  std::string p_string;
  unsigned p_size;
};

typedef Atkins::Fixture::Epiface_thread<Test_handler> StringRPC;

/**
 * Content of C strings is transferred correctly.
 */
TEST_F(StringRPC, InSimpleString)
{
  char const *teststr = "Hello World";
  EXPECT_EQ(0, scap()->in_simple_str(teststr));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  EXPECT_EQ(11U, handler().p_string.size());
}

/**
 * Empty C strings are transferred correctly.
 */
TEST_F(StringRPC, InEmptyString)
{
  EXPECT_EQ(0, scap()->in_simple_str(""));
  EXPECT_EQ(1U, handler().p_size);
  EXPECT_EQ(0U, handler().p_string.size());
}

/**
 * C strings of zero length yield an error.
 */
TEST_F(StringRPC, InNullString)
{
  EXPECT_EQ(-L4_EMSGTOOSHORT, scap()->in_simple_str(L4::Ipc::String<>(0, "")));
}

/**
 * C strings may contain a zero character.
 */
TEST_F(StringRPC, InStringWithNull)
{
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::String<>(4, "a\0b")));
  EXPECT_EQ(3U, handler().p_string.size());
  EXPECT_STREQ("a\0b", handler().p_string.c_str());
}

/**
 * C strings without zero termination are zero-terminated when received
 * by the server.
 */
TEST_F(StringRPC, InStringWithoutNullTermination)
{
  EXPECT_EQ(0, scap()->in_simple_str(L4::Ipc::String<>(3, "xyz")));
  EXPECT_EQ(2U, handler().p_string.size());
  EXPECT_STREQ("xy", handler().p_string.c_str());
}

/**
 * C strings that do not fit into the UTCB yield an error.
 */
TEST_F(StringRPC, InOversizedString)
{
  // Hand-craft a message with an oversized string.
  l4_msg_regs_t *mr = l4_utcb_mr();
  mr->mr[0] = Test_iface::Rpcs::Rpc<Test_iface::in_simple_str_t>::Opcode;
  mr->mr[1] = ~1UL;

  l4_msgtag_t msg = l4_ipc_call(scap().cap(), l4_utcb(),
                                l4_msgtag(Test_iface::Protocol, 2, 0, 0),
                                L4_IPC_NEVER);
  ASSERT_EQ(-L4_EMSGTOOSHORT, l4_error(msg));

}

/**
 * Empty C strings returned by the server are transferred correctly.
 */
TEST_F(StringRPC, OutEmptyString)
{
  handler().p_string = "";
  handler().p_string += '\0';
  char buf[60];
  L4::Ipc::String<char> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_simple_str(ret));
  EXPECT_EQ(1U, ret.length);
  EXPECT_EQ('\0', ret.data[0]);
}

/**
 * C strings of zero length returned by the server are transferred correctly.
 */
TEST_F(StringRPC, OutNullString)
{
  handler().p_string = "";
  char buf[60];
  L4::Ipc::String<char> ret(sizeof(buf), buf);
  EXPECT_EQ(-L4_EMSGTOOSHORT, scap()->out_simple_str(ret));
}

/**
 * C strings returned by the server may contain a zero character.
 */
TEST_F(StringRPC, OutNullTerminatedString)
{
  handler().p_string = "Goodbye Friends.";
  handler().p_string += '\0';
  char buf[60];
  L4::Ipc::String<char> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_simple_str(ret));
  ASSERT_EQ(17U, ret.length);
  EXPECT_EQ(handler().p_string, std::string(ret.data, ret.length));
  EXPECT_EQ(handler().p_string, std::string(buf, ret.length));
}

/**
 * C strings returned by the server without null-termination are
 * null-terminated when received by the client.
 */
TEST_F(StringRPC, OutStringWithoutNullTermination)
{
  handler().p_string = "xxyyzz";
  char buf[60];
  L4::Ipc::String<char> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_simple_str(ret));
  ASSERT_EQ(6U, ret.length);
  EXPECT_STREQ("xxyyz", ret.data);
  EXPECT_EQ(0, memcmp("xxyyz\0", buf, 6));
}

/**
 * An optional string parameter may be left out.
 */
TEST_F(StringRPC, OptInStringNotGiven)
{
  L4::Ipc::Opt<L4::Ipc::String<>> in;
  EXPECT_EQ(0, scap()->in_opt_str(false, in));
}

/**
 * An optional string is transferred to the server correctly when given.
 */
TEST_F(StringRPC, DISABLED_OptInStringGiven)
{
  L4::Ipc::Opt<L4::Ipc::String<>> in;
  char const *teststr = "foobar";
  in = L4::Ipc::String<>(teststr);
  in.set_valid(true);
  ASSERT_EQ(7U, in->length);
  ASSERT_EQ(0, scap()->in_opt_str(true, in));
  EXPECT_STREQ(teststr, handler().p_string.c_str());
  EXPECT_EQ(6U, handler().p_string.size());
}

/**
 * An optional out string may be omitted by the server
 */
TEST_F(StringRPC, DISABLED_OptOutStringNotGiven)
{
  char buf[60] = "x";
  L4::Ipc::String<char> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_opt_str(false, ret));
  EXPECT_EQ('x', buf[0]);
}

/**
 * An optional output string is transferred correctly to the client when given.
 */
TEST_F(StringRPC, OptOutStringGiven)
{
  handler().p_string = "Westward ho";
  handler().p_string += '\0';
  char buf[60] = "x";
  L4::Ipc::String<char> ret(sizeof(buf), buf);
  EXPECT_EQ(0, scap()->out_opt_str(true, ret));
  ASSERT_EQ(12U, ret.length);
  EXPECT_GT((l4_addr_t) ret.data, (l4_addr_t) l4_utcb_mr());
  EXPECT_LT((l4_addr_t) ret.data,
            (l4_addr_t) l4_utcb_mr() + sizeof(l4_msg_regs_t));
  EXPECT_STREQ("x", buf);
}
