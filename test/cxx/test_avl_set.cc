/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *            Philipp Eppelt <philipp.eppelt@kernkonzept.com>
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
#include <set>

#include <l4/sys/err.h>
#include <l4/cxx/avl_set>

#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>

#include "tracking_alloc.h"
#include "item"

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

// *** AVL set constructor, copy-constructor, and destructor tests ***********

using Container_test::Item;
using Item_set = cxx::Avl_set<Item, cxx::Lt_functor<Item>, TrackingAlloc>;

/**
 * Test fixture for AVL set tests using a non-trivial Item.
 */
class TestAvlCtors : public Test_track_alloc
{
protected:
  Item_set *avl_items;

public:
  void SetUp()
  {
    Test_track_alloc::SetUp();
    avl_items = new Item_set;

    EXPECT_EQ(avl_items->begin(), avl_items->end())
      << "Initial Item_set is empty.";

    EXPECT_TRUE(Item::item_address.empty()) << "No item addresses are stored.";
  }

  void TearDown()
  {
    delete avl_items;
    Test_track_alloc::TearDown();

    EXPECT_TRUE(Item::item_address.empty())
      << "All item addresses were removed.";
  }
};

/**
 * An AVL set creates a copy of the item to insert and maintains the copy
 * independent of the original instance.
 *
 * \see cxx::Avl_set
 */
TEST_F(TestAvlCtors, InsertCopiesItem)
{
    {
      Item ite;
      EXPECT_EQ(0, avl_items->insert(ite).second)
        << "The item is successfully inserted into the AVL set.";

      EXPECT_EQ(2U, Item::item_address.size())
        << "Avl_set.insert invoked the copy-constructor of Item.";

      EXPECT_EQ(*avl_items->begin(), ite)
        << "The item in the AVL set is a copy of the local item.";
    }

  EXPECT_EQ(1U, Item::item_address.size())
    << "The original item was destroyed at the end of the scope, the copy is"
       " still present.";
}

/**
 * An AVL set creates a copy of the item to insert and deletes the copy, if the
 * insertion fails.
 *
 * \see cxx::Avl_set
 */
TEST_F(TestAvlCtors, FailedInsertRemovesCopyAgain)
{
  Item ite;
  EXPECT_EQ(0, avl_items->insert(ite).second)
    << "The item is successfully inserted into the AVL set.";

  EXPECT_EQ(2U, Item::item_address.size())
    << "Avl_set.insert invoked the copy-constructor of Item.";

  EXPECT_NE(avl_items->end(), avl_items->find(ite))
    << "The inserted item is found in the AVL set.";

  EXPECT_EQ(-Int_set::E_exist, avl_items->insert(ite).second)
    << "The item is already in the AVL set.";

  EXPECT_EQ(2U, Item::item_address.size())
    << "The number of constructed items does not change after an unsuccessful"
       " insertion in an AVL set.\n";
}

/**
 * On removal of an element the AVL set invokes the destructor of said element.
 *
 * \see cxx::Avl_set
 */
TEST_F(TestAvlCtors, RemoveDestroysCopiedItem)
{
  Item ite;
  EXPECT_EQ(0, avl_items->insert(ite).second)
    << "The item is successfully inserted into the AVL set.";

  EXPECT_EQ(2U, Item::item_address.size())
    << "Avl_set.insert invoked the copy-constructor of Item.";

  EXPECT_L4OK(avl_items->remove(ite)) << "Remove item from the AVL set.";
  EXPECT_EQ(1U, Item::item_address.size())
    << "Avl_set.remove invoked item's destructor.";
}

/**
 * On destruction of an AVL set, the destructor of all stored elements is
 * invoked.
 *
 * \see cxx::Avl_set
 */
TEST_F(TestAvlCtors, DestructorDeletesAllCopies)
{
  unsigned const Number_of_items = 5;
  std::vector<Item> v_ite(Number_of_items);

  for (auto &ite : v_ite)
    EXPECT_EQ(0, avl_items->insert(ite).second)
      << "The item is successfully inserted into the AVL set.";

  EXPECT_EQ(Number_of_items * 2, Item::item_address.size())
    << "Each item was copied on insertion.";

  avl_items->~Avl_set();

  EXPECT_EQ(Number_of_items, Item::item_address.size())
    << "All copies were destroyed in the fire.";
}
