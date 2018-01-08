/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Marius Melzer <marius.melzer@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/atkins/l4_assert>
#include <l4/atkins/tap/main>
#include "throwexc.h"

using namespace Throw_exc;

/**
 * An exception thrown in a shared library is caught in the application code
 * by a catch all statement.
 */
TEST(ExceptionHandling, CatchAll)
{
  bool caught_exc = false;

  try
    {
      throw_str_exc();
    }
  catch (...)
    {
      caught_exc = true;
    }

  EXPECT_TRUE(caught_exc)
    << "String exception is caught via open catch.";
}

/**
 * An exception thrown in a shared library is caught in the application
 * code by a catch on the concrete exception type.
 */
TEST(ExceptionHandling, CatchCorrectType)
{
  bool caught_str_exc = false;

  try
    {
      throw_str_exc();
    }
  catch (char const *str)
    {
      caught_str_exc = true;
    }

  EXPECT_TRUE(caught_str_exc)
    << "String exception is caught via string catch.";
}

/**
 * An exception thrown in a shared library is not caught in the application
 * code by a catch on the incorrect exception type.
 */
TEST(ExceptionHandling, DontCatchWrongType)
{
  bool caught_int_exc = false;

  try
    {
      try
        {
          throw_str_exc();
        }
      catch (int exc)
        {
          caught_int_exc = true;
        }
    }
  catch (char const *str)
    {}
  EXPECT_FALSE(caught_int_exc)
    << "String exception is not caught via int catch.";
}

/**
 * An exception thrown in a shared library is caught in the application
 * code via a catch on the exceptions class.
 */
TEST(ExceptionHandling, CatchObject)
{
  bool caught_exc = false;

  try
    {
      throw_base_exc();
    }
  catch (Base_exc exc)
    {
      caught_exc = true;
    }

  EXPECT_TRUE(caught_exc)
    << "Base class object exception is caught via base class catch.";
}

/**
 * An exception of a derived class thrown in a shared library is caught in the
 * application code by a catch on the base class.
 */
TEST(ExceptionHandling, CatchDerivedObject)
{
  bool caught_exc = false;

  try
    {
      throw_derived_exc();
    }
  catch (Base_exc exc)
    {
      caught_exc = true;
    }

  EXPECT_TRUE(caught_exc)
    << "Derived class object exception is caught via base class catch.";
}

/**
 * An exception of a derived class - hidden from the application code via an
 * anonymous namespace - thrown in a shared library is caught in the
 * application code by a catch on the base class.
 */
TEST(ExceptionHandling, CatchHiddenDerivedObject)
{
  bool caught_exc = false;

  try
    {
      throw_hidden_derived_exc();
    }
  catch (Base_exc exc)
    {
      caught_exc = true;
    }

  EXPECT_TRUE(caught_exc)
    << "Derived class (in anonymous namespace) object exception is caught via"
       "base class catch.";
}

/**
 * Helper class which sets a flag passed at construction time when its
 * destructor was executed. A member function throws an exception when invoked.
 */
class Destructor_helper
{
private:
  bool *destructor_called;

public:
  Destructor_helper(bool *called) { destructor_called = called; }

  void
  throw_exc()
  {
    throw_str_exc();
  }

  ~Destructor_helper() { *destructor_called = true; }
};

/**
 * An exception thrown in a shared library triggered by a class member function
 * does not intervene with the normal destructor call on scope exit.
 *
 * \see Destructor_helper
 */
TEST(ExceptionHandling, DestructorCalledDespiteException)
{
  bool destructor_called = false;

  {
    Destructor_helper helper(&destructor_called);

    try
      {
        helper.throw_exc();
      }
    catch (...)
      {}

    EXPECT_FALSE(destructor_called)
      << "The destructor of the object is not called after an exception in a "
         "member function.";
  }

  EXPECT_TRUE(destructor_called)
      << "The destructor of the object is called after the object goes out of "
      << "scope.";
}
