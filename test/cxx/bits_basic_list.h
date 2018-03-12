/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
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

TYPED_TEST_P(BasicListTest, NewListIsEmpty)
{
  typename TypeParam::List l;
  EXPECT_TRUE(l.empty());
}

TYPED_TEST_P(BasicListTest, ListWithElementIsNotEmpty)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;
  l.push_front(&e);
  EXPECT_FALSE(l.empty());
}

TYPED_TEST_P(BasicListTest, ClearOnEmptyList)
{
  typename TypeParam::List l;
  l.clear();
  EXPECT_TRUE(l.empty());
}

TYPED_TEST_P(BasicListTest, ClearLeavesListEmpty)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;
  l.push_front(&e);
  l.clear();
  EXPECT_TRUE(l.empty());
}

TYPED_TEST_P(BasicListTest, BeginOnEmptyList)
{
  typename TypeParam::List l;
  EXPECT_EQ(l.begin(), l.end());
  EXPECT_EQ(*(l.begin()), nullptr);
}

TYPED_TEST_P(BasicListTest, ConstBeginOnEmptyList)
{
  typename TypeParam::List l;
  auto const begin = l.begin();
  auto const end = l.end();
  EXPECT_EQ(begin, end);
}

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

