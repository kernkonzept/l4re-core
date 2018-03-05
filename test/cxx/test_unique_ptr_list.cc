/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/cxx/unique_ptr_list>

#include <l4/atkins/tap/main>

#include "bits_smart_ptr_list.h"

static int last_destructed = -1;

struct Simple_elem : public cxx::Unique_ptr_list_item<Simple_elem>
{
  explicit Simple_elem(int ident) : id(ident) {}

  virtual ~Simple_elem()
  {
    last_destructed = id;
  }

  int id;
};

struct SimpleUniqueList
{
  using List = cxx::Unique_ptr_list<Simple_elem>;
  using Elem = Simple_elem;

  static cxx::unique_ptr<Elem> make_elem(int id = 0)
  { return cxx::make_unique<Elem>(id); }

};

typedef testing::Types<SimpleUniqueList> ListTypes;

INSTANTIATE_TYPED_TEST_CASE_P(Basic, BasicSmartListTest, ListTypes);

/**
 * Allocation tests for unique_ptr_lists.
 */
struct UniqueListTest : public testing::Test
{
  void SetUp()
  {
    last_destructed = -1;
  }

  static cxx::unique_ptr<Simple_elem> make_elem(int id = 0)
  { return cxx::make_unique<Simple_elem>(id); }

  cxx::Unique_ptr_list<Simple_elem> l;
};

/**
 * Push_front() moves the element into the list without destroying it.
 */
TEST_F(UniqueListTest, ElementNotDestructedAfterPushFront)
{
  for (int i = 0; i < 3; ++i)
    {
      auto e = make_elem(564 + i);
      EXPECT_EQ(-1, last_destructed)
        << "No destructor was called after the element has been constructed.";

      l.push_front(cxx::move(e));
      EXPECT_EQ(-1, last_destructed)
        << "The element has not been destructed after push_front() to the list.";
    }
}

/**
 * Push_back() moves the element into the list without destroying it.
 */
TEST_F(UniqueListTest, ElementNotDestructedAfterPushBack)
{
  for (int i = 0; i < 3; ++i)
    {
      auto e = make_elem(564 + i);
      EXPECT_EQ(-1, last_destructed)
        << "No destructor was called after the element has been constructed.";

      l.push_back(cxx::move(e));
      EXPECT_EQ(-1, last_destructed)
        << "The element has not been destructed after push_back() to the list.";
    }
}

/**
 * Pop_front() moves the element out of the list.
 */
TEST_F(UniqueListTest, ElementCanBeDestructedAfterPopFront)
{
  for (int i = 10; i < 13; ++i)
    l.push_back(make_elem(i));

  EXPECT_EQ(-1, last_destructed)
    << "No element was destructed while the list was filled with many elements.";
  for (int i = 10; i < 13; ++i)
    {
      l.pop_front();
      EXPECT_EQ(i, last_destructed)
        << "The element is destroyed after being removed from the list.";
    }
}

/**
 * An element in the list is destructed when the list is destructed.
 */
TEST(UniqueListDestructionTest, ElementIsDestructedWithList)
{
  last_destructed = -1;
  {
    cxx::Unique_ptr_list<Simple_elem> l;
    l.push_back(cxx::make_unique<Simple_elem>(6678));
    EXPECT_EQ(-1, last_destructed)
      << "The element is not destructed after inserting to the list.";
  }

  EXPECT_EQ(6678, last_destructed)
    << "The element in the list is destructed after list goes out of scope.";
}
