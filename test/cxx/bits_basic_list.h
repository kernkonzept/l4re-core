/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Basic tests for generic lists.
 *
 * The tests defined here are included in both test_hlist.cc and
 * test_slist.cc and instanciated there for different types of double
 * and single linked lists, respectively.
 */

#include <gtest/gtest.h>

/**
 * \internal
 * Tests common for all list implementations based on Bits::Basic_list
 *
 * \tparam LIST Struct defining the types for the list and its elements.
 */
template <typename LIST_DESC>
struct BasicListTest : public testing::Test
{
};

TYPED_TEST_CASE_P(BasicListTest);

/**
 * A newly created list is empty.
 */
TYPED_TEST_P(BasicListTest, NewListIsEmpty)
{
  typename TypeParam::List l;
  EXPECT_TRUE(l.empty());
}

/**
 * A list containing one element is not empty.
 */
TYPED_TEST_P(BasicListTest, ListWithElementIsNotEmpty)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;
  l.push_front(&e);
  EXPECT_FALSE(l.empty());
}

/**
 * An empty list is empty after clear() was called.
 */
TYPED_TEST_P(BasicListTest, ClearOnEmptyList)
{
  typename TypeParam::List l;
  l.clear();
  EXPECT_TRUE(l.empty());
}

/**
 * A list is empty after clear() was called.
 */
TYPED_TEST_P(BasicListTest, ClearLeavesListEmpty)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;
  l.push_front(&e);
  l.clear();
  EXPECT_TRUE(l.empty());
}

/**
 * begin() and end() iterators are equal for an empty list and begin() points
 * to nullptr.
 */
TYPED_TEST_P(BasicListTest, BeginOnEmptyList)
{
  typename TypeParam::List l;
  EXPECT_EQ(l.begin(), l.end());
  EXPECT_EQ(*(l.begin()), nullptr);
}

/**
 * Constant begin() and end() iterators are equal for an empty list.
 */
TYPED_TEST_P(BasicListTest, ConstBeginOnEmptyList)
{
  typename TypeParam::List l;
  auto const begin = l.begin();
  auto const end = l.end();
  EXPECT_EQ(begin, end);
}

/**
 * push_front() inserts elements at the beginning of a list.
 */
TYPED_TEST_P(BasicListTest, PushFrontInsertsAtTheBeginning)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    {
      l.push_front(ele + i);
      EXPECT_EQ(ele + i, l.front());
    }

}

/**
 * add() inserts elements at the beginning of a list.
 */
TYPED_TEST_P(BasicListTest, AddInsertsAtTheBeginning)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    {
      l.add(ele + i);
      EXPECT_EQ(ele + i, l.front());
    }

}

/**
 * push_front() does not change the order of previously inserted list elements.
 */
TYPED_TEST_P(BasicListTest, PushFrontDoesNotChangeOrder)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    {
      l.push_front(ele + i);
      int c = i;
      for (auto *e : l)
        {
          EXPECT_EQ(ele + c, e);
          ++c;
        }
    }

}

/**
 * A list can be iterated using the for-each loop.
 */
TYPED_TEST_P(BasicListTest, ForEachIterate)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  int i = 0;
  for (auto *e : l)
    {
      EXPECT_EQ(ele + i, e);
      ++i;
    }
}

/*TYPED_TEST_P(BasicListTest, ForEachIteratorFromElement)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  for (int start = 0; start < 10; ++start)
    {
      int i = start;
      for (auto const it = l.iter(ele + start); it != l.end(); ++it)
        {
          EXPECT_EQ(ele + i, *it);
          ++i;
        }
    }
}
*/

/**
 * pop_front() is the reverse operation of push_front().
 */
TYPED_TEST_P(BasicListTest, PushFrontReverseOfPopFront)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(ele + i, l.pop_front());

  EXPECT_TRUE(l.empty());
}

/**
 * Erasing the begin of a list removes the first element and returns an
 * iterator to the new first element of the list.
 */
TYPED_TEST_P(BasicListTest, EraseFront)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  auto it = l.erase(l.begin());
  EXPECT_EQ(it, l.begin());
  EXPECT_EQ(*it, ele + 1);
}

/**
 * An arbitrary iterator between [begin, end) can be passed to erase to remove
 * the element referenced by the iterator. The element is properly removed and
 * the list can still be iterated.
 */
TYPED_TEST_P(BasicListTest, EraseMiddle)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  auto it = ++(++(l.begin()));
  it = l.erase(it);
  EXPECT_EQ(it, ++(++(l.begin())));
  it = ++l.begin();
  EXPECT_EQ(*it, ele + 1);
  ++it;
  EXPECT_EQ(*it, ele + 3);
  ++it;
  EXPECT_EQ(*it, ele + 4);
}

/**
 * The last element of a list can be removed without modifying unrelated
 * elements.
 */
TYPED_TEST_P(BasicListTest, EraseEnd)
{
  typename TypeParam::Elem ele[3];
  typename TypeParam::List l;

  for (int i = 2; i >= 0; --i)
    l.push_front(ele + i);

  auto it = ++(++(l.begin()));
  it = l.erase(it);
  EXPECT_EQ(it, ++(++(l.begin())));
  it = ++l.begin();
  EXPECT_EQ(*it, ele + 1);
  ++it;
  EXPECT_EQ(it, l.end());
}


REGISTER_TYPED_TEST_CASE_P(BasicListTest,
                           NewListIsEmpty, ListWithElementIsNotEmpty,
                           ClearOnEmptyList, ClearLeavesListEmpty,
                           BeginOnEmptyList, ConstBeginOnEmptyList,
                           AddInsertsAtTheBeginning,
                           PushFrontInsertsAtTheBeginning,
                           PushFrontDoesNotChangeOrder, ForEachIterate,
                           PushFrontReverseOfPopFront,
                           EraseFront, EraseMiddle, EraseEnd);

