/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Alexander Warg <alexander.warg@kernkonzept.com>
 *            Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test Ned's direct control interface.
 */
#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/ned/cmd_control>
#include <l4/re/env>

class NedTest : public ::testing::Test
{
public:
  // invoked before running a test
  void SetUp() override
  {
    cmd = L4Re::Env::env()->get_cap<L4Re::Ned::Cmd_control>("cmd");
    EXPECT_TRUE(cmd);
  }

  // invoked after the test finished
  void TearDown() override {}

  L4::Cap<L4Re::Ned::Cmd_control> cmd;
};


/**
 * Ned returns an error when asked to execute a non-existing command.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, nil_function)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  ASSERT_EQ(-L4_EIO, cmd->execute("non_existing()"));
}

/**
 * Ned returns an error when a command contains a syntax error.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, syntax_error)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  ASSERT_EQ(-L4_EINVAL, cmd->execute("syntax error"));
}

/**
 * Ned executes a function present in its global Lua state without error.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, test_func)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  ASSERT_EQ(0, cmd->execute("test()"));
}

/**
 * Ned's interface provides a string representation of the return value of the
 * executed Lua code.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, test_func_result_ret_1)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return 1", &result));
  ASSERT_EQ(0, memcmp(result.data, "1", result.length));
}

/**
 * Ned's interface provides a string representation of the returned table of the
 * executed Lua code.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, test_func_result_ret_table)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return { a = 'd' }", &result));
  ASSERT_GT(result.length, 5UL);
  ASSERT_EQ(0, memcmp(result.data, "table:", 6));
}

/**
 * Ned's interface provides access to variables in Ned's global Lua state.
 * When the returned string does not fit into the UTCB, the error is silently
 * ignored.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, test_func_result_ret_long_string)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return overly_long_string", &result));
}

/**
 * Ned's interface provides a string representation of the return value of the
 * invoked function present in Ned's global Lua state.
 *
 * \see L4Re::Ned::Cmd_control::execute
 */
TEST_F(NedTest, test_func_result)
{
  TAP_COMP_FUNC("Ned", "L4Re::Ned::Cmd_control.execute");

  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return test()", &result));
  ASSERT_EQ(0, memcmp(result.data, "hooo", result.length));
}

