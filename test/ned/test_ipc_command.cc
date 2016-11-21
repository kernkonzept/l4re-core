
#include <l4/atkins/tap/main>
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


TEST_F(NedTest, nil_function)
{
  ASSERT_EQ(-L4_EIO, cmd->execute("non_existing()"));
}

TEST_F(NedTest, syntax_error)
{
  ASSERT_EQ(-L4_EINVAL, cmd->execute("syntax error"));
}

TEST_F(NedTest, test_func)
{
  ASSERT_EQ(0, cmd->execute("test()"));
}

TEST_F(NedTest, test_func_result_ret_1)
{
  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return 1", &result));
  ASSERT_EQ(0, memcmp(result.data, "1", result.length));
}

TEST_F(NedTest, test_func_result_ret_table)
{
  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return { a = 'd' }", &result));
  ASSERT_GT(result.length, 5);
  ASSERT_EQ(0, memcmp(result.data, "table:", 6));
}

TEST_F(NedTest, test_func_result_ret_long_string)
{
  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return overly_long_string", &result));
}

TEST_F(NedTest, test_func_result)
{
  char buf[60];
  L4::Ipc::String<char> result(sizeof(buf), buf);
  ASSERT_EQ(0, cmd->execute("return test()", &result));
  ASSERT_EQ(0, memcmp(result.data, "hooo", result.length));
}

