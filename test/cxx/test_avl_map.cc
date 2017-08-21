/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test AVL map data types, bracket operator, and insert function.
 */
#include <string>
#include <vector>

#include <l4/cxx/avl_map>
#include <l4/atkins/tap/main>

#include "tracking_alloc.h"

typedef Test_track_alloc TestAvlMap;

// All tests use a standard compare function and the observing allocator
// to check for memory leaks.
template <typename KEY, typename DATA>
using Test_map = cxx::Avl_map<KEY, DATA, cxx::Lt_functor, TrackingAlloc>;

/**
 * The bracket operator inserts a new integer-type key into an empty map and
 * returns the value of the inserted key.
 *
 * \see cxx::Avl_map.operator []
 */
TEST_F(TestAvlMap, ReadWriteIntInt)
{
  Test_map<int, int> map;

  map[2] = -2;

  EXPECT_EQ(-2, map[2]);
}

/**
 * The bracket operator inserts and returns a key-data pair for string keys.
 *
 * \see cxx::Avl_map.operator []
 */
TEST_F(TestAvlMap, ReadWriteStringInt)
{
  Test_map<std::string, int> map;

  map["foo"] = 6738;
  map["fooo"] = 0;

  EXPECT_EQ(6738, map["foo"]);
  EXPECT_EQ(0, map["fooo"]);

  map["foo"] -= 2;

  EXPECT_EQ(6736, map["foo"]);
}

/**
 * The AVL map can manage std::vector as data type.
 *
 * \see cxx::Avl_map.operator []
 */
TEST_F(TestAvlMap, ReadWriteIntVector)
{
  Test_map<int, std::vector<char>> map;

  map[0].push_back('x');
  map[-1].push_back('y');
  map[0].push_back('y');

  EXPECT_EQ(2U, map[0].size());
  EXPECT_EQ(1U, map[-1].size());
  EXPECT_EQ(0U, map[1].size());
}

/**
 * When inserting a key that already exist, the operation fails and the
 * existing entry is returned.
 *
 * \see cxx::Avl_map.insert
 */
TEST_F(TestAvlMap, InsertIntString)
{
  using Map = Test_map<int, std::string>;
  Map map;

  EXPECT_EQ(0, map.insert(0, "pickle").second);
  EXPECT_EQ(-Map::E_exist, map.insert(0, "buckle").second);
  EXPECT_EQ("pickle", map.insert(0, "buckle").first->second);

  EXPECT_EQ("pickle", map[0]);

  map[100] = "huckleberry";

  EXPECT_EQ(-Map::E_exist, map.insert(100, "buckle").second);
  EXPECT_EQ("huckleberry", map.insert(100, "buckle").first->second);
}
