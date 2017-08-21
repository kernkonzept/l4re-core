/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test AVL set properties.
 */
#include <map>
#include <algorithm>
#include <memory>

#include <l4/sys/err.h>
#include <l4/cxx/avl_set>

#include <l4/atkins/tap/main>

#include "tracking_alloc.h"

typedef cxx::Avl_set<int, cxx::Lt_functor<int>, TrackingAlloc > Int_set;

struct TestAvlSet : public Test_track_alloc
{
  Int_set *tree;

  virtual void SetUp()
  {
    tree = new Int_set();
  }

  virtual void TearDown()
  {
    delete tree; // manual cleaup, so the allocator can be checked
  }
};

/**
 * AVL set error codes are compatible to L4Re error codes.
 *
 * \see cxx::Base_avl_set
 */
TEST_F(TestAvlSet, ErrorCodesAreCompatible)
{
  EXPECT_EQ(l4_error_code_t(Int_set::E_noent), L4_ENOENT);
  EXPECT_EQ(l4_error_code_t(Int_set::E_exist), L4_EEXIST);
  EXPECT_EQ(l4_error_code_t(Int_set::E_nomem), L4_ENOMEM);
  EXPECT_EQ(l4_error_code_t(Int_set::E_inval), L4_EINVAL);
}

/**
 * begin() and end() iterators are equal for an empty AVL set.
 *
 * \see cxx::Avl_set.begin, cxx::Avl_set.end, cxx::Avl_set.rbegin,
 *      cxx::Avl_set.rend
 */
TEST_F(TestAvlSet, IteratorsOnEmpty)
{
  EXPECT_EQ(tree->begin(), tree->end());
  EXPECT_EQ(tree->rbegin(), tree->rend());
}

/**
 * Elements can be inserted into an AVL set.
 *
 * \see cxx::Avl_set.insert
 */
TEST_F(TestAvlSet, Insert)
{
  EXPECT_EQ(0, tree->insert(1).second);
  EXPECT_EQ(0, tree->insert(2).second);
  EXPECT_EQ(0, tree->insert(0).second);
}

/**
 * Elements can be inserted into an AVL set only once. If the element is
 * already in the set -E_exist and the existing element is returned.
 *
 * \see cxx::Avl_set.insert
 */
TEST_F(TestAvlSet, InsertDuplicate)
{
  auto ret = tree->insert(42);

  EXPECT_EQ(42, *(ret.first));
  EXPECT_EQ(0, ret.second);

  ret = tree->insert(42);

  EXPECT_EQ(42, *(ret.first));
  EXPECT_EQ(-Int_set::E_exist, ret.second);
}

/**
 * Elements in the set can be removed.
 *
 * \see cxx::Avl_set.remove
 */
TEST_F(TestAvlSet, RemovePartial)
{
  std::vector<int> ids { 6, 9, 0, 1, 4 };

  for (int i : ids)
    EXPECT_EQ(0, tree->insert(i).second);

  for (int i = 0; i < 3; ++i)
    EXPECT_EQ(0, tree->remove(ids[i]));
}

/**
 * When all elements in the set are removed the set is empty.
 *
 * \see cxx::Avl_set.remove
 */
TEST_F(TestAvlSet, RemoveAll)
{
  std::vector<int> ids { 6, 9, 0, 1, 4 };

  for (int i : ids)
    EXPECT_EQ(0, tree->insert(i).second);

  for (int i : ids)
    EXPECT_EQ(0, tree->remove(i));

  EXPECT_EQ(tree->begin(), tree->end());
}

/**
 * An element can only be removed once from an AVL set. Attempts to remove
 * elements not in the set receive an -E_noent error.
 *
 * \see cxx::Avl_set.remove
 */
TEST_F(TestAvlSet, RemoveDuplicate)
{
  EXPECT_EQ(-Int_set::E_noent, tree->remove(0));

  EXPECT_EQ(0, tree->insert(-2222).second);

  EXPECT_EQ(0, tree->remove(-2222));
  EXPECT_EQ(-Int_set::E_noent, tree->remove(-2222));
  EXPECT_EQ(-Int_set::E_noent, tree->remove(34));
}

/**
 * Elements inserted into an AVL set are found via find_node() and valid().
 * find_node() requests for elements not in the set receive an invalid node.
 *
 * \see cxx::Avl_set.find_node
 */
TEST_F(TestAvlSet, FindNode)
{
  EXPECT_EQ(0, tree->insert(123).second);

  auto n = tree->find_node(123);
  EXPECT_TRUE(n);
  EXPECT_TRUE(n.valid());
  EXPECT_EQ(123, *n);

  n = tree->find_node(122);
  EXPECT_FALSE(n);
  EXPECT_FALSE(n.valid());
}

/**
 * An AVL set can be copied via the copy-constructor.
 *
 * \see cxx::Avl_set
 */
TEST_F(TestAvlSet, CopyTree)
{
  std::unique_ptr<Int_set> copied;
  std::vector<int> ids { -90, 34, 35, 36 };

    {
      std::unique_ptr<Int_set> p {new Int_set()};

      for (int i : ids)
        EXPECT_EQ(0, p->insert(i).second);

      copied.reset(new Int_set(*p.get()));
    }

  for (int i : ids)
    {
      auto n = copied->find_node(i);
      EXPECT_TRUE(n);
      EXPECT_EQ(i, *n);
    }
}

/**
 * An empty AVL set can be copied via the copy-constructor.
 *
 * \see cxx::Avl_set
 */
TEST_F(TestAvlSet, CopyEmptyTree)
{
  std::unique_ptr<Int_set> p {new Int_set(*tree)};

  EXPECT_EQ(p->begin(), p->end());
}
