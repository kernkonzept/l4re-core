/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Basic tests for generic lists with smaprt pointer connectors.
 */

#include <l4/cxx/type_traits>
#include <gtest/gtest.h>

/**
 * \internal
 * Tests common for all list implementations based on Bits::Smart_ptr_list.
 *
 * \tparam LIST  Struct defining the types for the list and its elements.
 */
template <typename LIST_DESC>
struct BasicSmartListTest : public testing::Test
{
};

TYPED_TEST_CASE_P(BasicSmartListTest);

/**
 * A newly created list is empty.
 */
TYPED_TEST_P(BasicSmartListTest, NewListIsEmpty)
{
  typename TypeParam::List l;
  EXPECT_TRUE(l.empty())
    << "The newly created list is empty.";
}

/**
 * A list where an element has been added with push_front() is not empty.
 */
TYPED_TEST_P(BasicSmartListTest, ListWithElementAtFrontIsNotEmpty)
{
  typename TypeParam::List l;
  auto e = TypeParam::make_elem();
  l.push_front(cxx::move(e));
  EXPECT_FALSE(l.empty())
    << "The list is not empty after an element has been added to the front.";
}

/**
 * A list where an element has been added with push_back() is not empty.
 */
TYPED_TEST_P(BasicSmartListTest, ListWithElementAtBackIsNotEmpty)
{
  typename TypeParam::List l;
  auto e = TypeParam::make_elem();
  l.push_front(cxx::move(e));
  EXPECT_FALSE(l.empty())
    << "The list is not empty after an element has been added to the back.";
}

/**
 * begin() and end() iterators are equal for an empty list and begin() points
 * to nullptr.
 */
TYPED_TEST_P(BasicSmartListTest, BeginOnEmptyList)
{
  typename TypeParam::List l;
  EXPECT_EQ(l.begin(), l.end())
    << "Begin and end of an empty list are the same.";
  EXPECT_EQ(*(l.begin()), nullptr)
    << "Begin of an empty list is equal to the nullptr.";
}

/**
 * Constant begin() and end() iterators are equal for an empty list.
 */
TYPED_TEST_P(BasicSmartListTest, ConstBeginOnEmptyList)
{
  typename TypeParam::List l;
  auto const begin = l.begin();
  auto const end = l.end();
  EXPECT_EQ(begin, end)
    << "Constant begin and end of an empty list are the same.";
}

/**
 * push_front() inserts elements at the beginning of a list.
 */
TYPED_TEST_P(BasicSmartListTest, PushFrontInsertsAtTheBeginning)
{
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    {
      l.push_front(TypeParam::make_elem(i));
      EXPECT_EQ(i, l.front()->id)
        << "The front element contains the last pushed element.";
    }

}

/**
 * push_front() does not change the order of previously inserted list elements.
 */
TYPED_TEST_P(BasicSmartListTest, PushFrontDoesNotChangeOrder)
{
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    {
      l.push_front(TypeParam::make_elem(i));
      int c = i;
      for (auto *e : l)
        {
          EXPECT_EQ(c, e->id)
            << "The element '" << c
            << "' remains at the position where it was inserted";
          ++c;
        }
    }

}

/**
 * A list can be iterated using the for-each loop.
 */
TYPED_TEST_P(BasicSmartListTest, ForEachIterate)
{
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(TypeParam::make_elem(i));

  int i = 0;
  for (auto *e : l)
    {
      EXPECT_EQ(i, e->id)
        << "The iterator at postion '" << i
        << "' contains the expected element.";
      ++i;
    }
}

/**
 * pop_front() is the reverse operation of push_front().
 */
TYPED_TEST_P(BasicSmartListTest, PushFrontReverseOfPopFront)
{
  typename TypeParam::List l;

  for (int i = 3; i >= 0; --i)
    {
      l.push_front(TypeParam::make_elem(i));
      ASSERT_EQ(i, l.pop_front()->id)
        << "The element returned by pop_front() is the recently inserted element.";
      ASSERT_TRUE(l.empty())
        << "The list is empty after removing the just inserted element.";
    }
}

/**
 * After many elements are pushed to the front, they can be removed
 * in reverse order with pop_front().
 */
TYPED_TEST_P(BasicSmartListTest, PushFrontPopMany)
{
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(TypeParam::make_elem(i));

  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(i, l.pop_front()->id)
      << "The element returned by pop_front() is the recently inserted element.";

  EXPECT_TRUE(l.empty())
    << "The list is empty after all elements have been removed again.";
}

/**
 * An element put into the empty element with push_back() can be retrived
 * with pop_front().
 */
TYPED_TEST_P(BasicSmartListTest, PushBackReturnAsPopFront)
{
  typename TypeParam::List l;

  for (int i = 3; i >= 0; --i)
    {
      l.push_back(TypeParam::make_elem(i));
      ASSERT_EQ(i, l.pop_front()->id)
        << "The element returned by pop_front() is the recently inserted element.";
      ASSERT_TRUE(l.empty())
        << "The list is empty after removing the just inserted element.";
    }
}

/**
 * After many elements are pushed to the back, they can be removed
 * in the same order with pop_front().
 */
TYPED_TEST_P(BasicSmartListTest, PushBackPopMany)
{
  typename TypeParam::List l;

  for (int i = 0; i < 10; ++i)
    l.push_back(TypeParam::make_elem(i));

  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(i, l.pop_front()->id)
      << "The element returned by pop_front() is the recently inserted element.";

  EXPECT_TRUE(l.empty())
    << "The list is empty after all elements have been removed again.";
}



REGISTER_TYPED_TEST_CASE_P(BasicSmartListTest,
                           NewListIsEmpty, ListWithElementAtFrontIsNotEmpty,
                           ListWithElementAtBackIsNotEmpty,
                           BeginOnEmptyList, ConstBeginOnEmptyList,
                           PushFrontInsertsAtTheBeginning,
                           PushFrontDoesNotChangeOrder, ForEachIterate,
                           PushFrontReverseOfPopFront, PushFrontPopMany,
                           PushBackReturnAsPopFront, PushBackPopMany
                          );
