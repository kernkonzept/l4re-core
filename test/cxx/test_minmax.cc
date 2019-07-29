/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Diego Machado Dias <diego.dias@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/atkins/tap/main>
#include <l4/cxx/minmax>

/**
 * Type parameterized fixture to iterate over list of ordered types.
 */
template <typename T> class MinMax : public ::testing::Test
{};

/**
 * Comparable implements comparison operators needed for using min/max (<, >).
 * It is used to show that min/max are compatible with non-integral types.
 */
class Comparable
{
public:
  Comparable(int i) : _item(i) {}

  /// Less-than (`<`) operator
  bool operator < (Comparable const &o) const noexcept
  { return _item < o._item; }

  /// Greater-than (`>`) operator
  bool operator > (Comparable const &o) const noexcept
  { return _item > o._item; }

  /// Equality (`==`) operator
  bool operator == (Comparable const &o) const noexcept
  { return _item == o._item; }

private:
  /// Value wrapped by Comparable
  int _item;
};

using OrderedTypes =
  ::testing::Types<char, short int, int, long, long long, Comparable>;
TYPED_TEST_CASE(MinMax, OrderedTypes);

/**
 * `cxx::min(a, b)` must return the minimum of `a` and `b`.
 */
TYPED_TEST(MinMax, Min)
{
  TypeParam min_v = 0;
  TypeParam max_v = 1;

  ASSERT_EQ(min_v, cxx::min(min_v, max_v)) << "First argument is smaller.";
  ASSERT_EQ(min_v, cxx::min(max_v, min_v)) << "Second argument is smaller.";
  ASSERT_EQ(max_v, cxx::min(max_v, max_v)) << "Both arguments are identical.";
};

/**
 * `cxx::max(a, b)` must return the maximum of `a` and `b`.
 */
TYPED_TEST(MinMax, Max)
{
  TypeParam min_v = 0;
  TypeParam max_v = 1;

  ASSERT_EQ(max_v, cxx::max(max_v, min_v)) << "First argument is larger.";
  ASSERT_EQ(max_v, cxx::max(min_v, max_v)) << "Second argument is larger.";
  ASSERT_EQ(max_v, cxx::max(max_v, max_v)) << "Both arguments are identical.";
};