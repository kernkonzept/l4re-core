/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Timo Nicolai <timo.nicolai@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * This file tests whether command line arguments and environment variables
 * specified in the Loader:start() Ned-script function are made available to
 * the newly created process.
 *
 * It depends on the Lua script 'arg.cfg' in the same directory.
 */
#include <gtest/gtest.h>

#include <l4/atkins/tap/tap>
#include <l4/re/env>
#include <l4/sys/debugger.h>

static std::vector<std::string> global_argv;
static std::map<std::string, std::string> global_envp;

/**
 * This task can access command line arguments specified in Loader:start().
 *
 * It is checked whether the string consisting of this task's program's name and
 * whitespace separated command line arguments given as the second argument to
 * the Loader:start() function that has spawned this task is consistent with
 * argc/argv.
 */
TEST(CorrectInvocation, ReceiveParams)
{
  std::string exp_prog_name("rom/test_arg");
  char const *const exp_params[] = {"-x", "-yz", "--long", "arg1", "arg2"};

  EXPECT_EQ(6u, global_argv.size()) << "Number of arguments is as expected.";

  EXPECT_EQ(exp_prog_name, global_argv[0]) << "Program name is as expected.";

  int i = 1;
  for (char const *param : exp_params)
    {
      EXPECT_EQ(global_argv[i], param) << "Argument " << i << " is as expected.";
      ++i;
    }
}

/**
 * This task can access Environment variables specified in Loader:start().
 *
 * It is checked whether the environment variables defined in the (optional)
 * third argument passed to the Loader:start() function that has spawned this
 * task are available to this task via getenv().
 */
TEST(CorrectInvocation, ReceiveEnvironment)
{
  char const *const exp_env[][2] = {{"ENV_VAR1", "env_var1"},
                                    {"ENV_VAR2", "env_var2"}};

  EXPECT_EQ(2u, global_envp.size())
    << "Number of environment variables is as expected.";

  for (auto entry : exp_env)
    {
      EXPECT_TRUE(global_envp.find(entry[0]) != global_envp.end())
        << "Environment variable '" << entry[0] << "' present.";
      EXPECT_EQ(entry[1], global_envp.at(entry[0]))
        << "Environment variable '" << entry[0] << "' has correct value.";
    }
}

/**
 * Performs setup similar to the main method in tap/main but does not handle
 * test command line options and saves all command line options and environment
 * variables into global vectors which are accessed during test execution.
 */
GTEST_API_ int
main(int argc, char **argv, char **envp)
{
  testing::InitGoogleTest(&argc, argv);

  for (int i = 0; i < argc; ++i)
    global_argv.push_back(argv[i]);

  for (char **env = envp; *env != nullptr; ++env)
    {
      std::string tmp(*env);
      size_t eq_pos = tmp.find('=');
      global_envp[tmp.substr(0, eq_pos)] = tmp.substr(eq_pos + 1);
    }

  L4Re::Util::Dbg::set_level(0);

  // Delete the default listener.
  testing::TestEventListeners &listeners =
    testing::UnitTest::GetInstance()->listeners();
  delete listeners.Release(listeners.default_result_printer());

  // Append our own tap listener.
  listeners.Append(new Atkins::Tap::Tap_listener(false));

  // set a meaningful name for debugging
  l4_debugger_set_object_name(L4Re::Env::env()->main_thread().cap(),
                              "gtest_main");

  int ret = RUN_ALL_TESTS();

  return ret;
}
