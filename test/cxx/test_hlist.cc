/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/hlist>

#include <l4/atkins/tap/main>

#include "bits_basic_list.h"

class Simple_elem : public cxx::H_list_item {};
class Typed_elem  : public cxx::H_list_item_t<Typed_elem> {};

struct SimpleHlist {
    typedef cxx::H_list<Simple_elem> List;
    typedef Simple_elem Elem;
};

struct TypedHlist {
    typedef cxx::H_list_t<Typed_elem> List;
    typedef Typed_elem Elem;
};

typedef testing::Types<SimpleHlist, TypedHlist> HlistTypes;

INSTANTIATE_TYPED_TEST_CASE_P(Basic, BasicListTest, HlistTypes);

template <typename LIST_DESC>
struct HListTest : public testing::Test
{
};

TYPED_TEST_CASE_P(HListTest);

TYPED_TEST_P(HListTest, InList)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;
  EXPECT_FALSE(l.in_list(&e));
  l.push_front(&e);
  EXPECT_TRUE(l.in_list(&e));
  l.remove(&e);
  EXPECT_FALSE(l.in_list(&e));
}

TYPED_TEST_P(HListTest, NotInListAfterPop)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;
  l.push_front(&e);
  EXPECT_TRUE(l.in_list(&e));
  l.pop_front();
  EXPECT_FALSE(l.in_list(&e));
}

TYPED_TEST_P(HListTest, InsertInEmpty)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;

  auto it = l.insert(&e, l.begin());
  EXPECT_EQ(&e, l.front());
  EXPECT_EQ(*it, &e);
}

TYPED_TEST_P(HListTest, InsertAtBeginning)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e1;
  typename TypeParam::Elem e2;

  l.push_front(&e1);
  auto it = l.insert(&e2, l.begin());
  EXPECT_EQ(&e1, l.front());
  EXPECT_EQ(*it, &e2);
  ++it;
  EXPECT_EQ(it, l.end());
  it = ++l.begin();
  EXPECT_EQ(*it, &e2);
}

TYPED_TEST_P(HListTest, InsertMiddle)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  auto it = l.insert(&insele, l.iter(e + 2));
  EXPECT_EQ(e, l.front());
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(e + 3, *it);
}

TYPED_TEST_P(HListTest, InsertAtFinal)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  auto it = l.insert(&insele, l.iter(e + 4));
  EXPECT_EQ(e, l.front());
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(l.end(), it);
}

TYPED_TEST_P(HListTest, InsertAtEnd)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  auto it = l.insert(&insele, l.end());
  EXPECT_EQ(&insele, l.front());
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(e, *it);
}

TYPED_TEST_P(HListTest, InsertAfterAtBeginning)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e1;
  typename TypeParam::Elem e2;

  l.push_front(&e1);
  auto it = l.insert_after(&e2, l.begin());
  EXPECT_EQ(&e1, l.front());
  EXPECT_EQ(*it, &e2);
  ++it;
  EXPECT_EQ(it, l.end());
  it = ++l.begin();
  EXPECT_EQ(*it, &e2);
}

TYPED_TEST_P(HListTest, InsertAfterMiddle)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  auto it = l.insert_after(&insele, l.iter(e + 2));
  EXPECT_EQ(e, l.front());
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(e + 3, *it);
}

TYPED_TEST_P(HListTest, InsertAfterAtFinal)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  auto it = l.insert_after(&insele, l.iter(e + 4));
  EXPECT_EQ(e, l.front());
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(l.end(), it);
}

TYPED_TEST_P(HListTest, InsertBeforeInEmpty)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e;

  l.insert_before(&e, l.begin());
  EXPECT_EQ(&e, l.front());
}

TYPED_TEST_P(HListTest, InsertBeforeAtBeginning)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e1;
  typename TypeParam::Elem e2;

  l.push_front(&e1);
  l.insert_before(&e2, l.begin());
  EXPECT_EQ(&e2, l.front());
  auto it = ++l.begin();
  EXPECT_EQ(*it, &e1);
  ++it;
  EXPECT_EQ(it, l.end());
}

TYPED_TEST_P(HListTest, InsertBeforeMiddle)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  l.insert_before(&insele, l.iter(e + 2));
  EXPECT_EQ(e, l.front());
  auto it = ++l.iter(e + 1);
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(e + 2, *it);
}

TYPED_TEST_P(HListTest, InsertBeforeAtFinal)
{
  typename TypeParam::List l;
  typename TypeParam::Elem e[5];

  for (int i = 4; i >= 0; --i)
    l.push_front(e + i);

  typename TypeParam::Elem insele;

  l.insert_before(&insele, l.iter(e + 4));
  EXPECT_EQ(e, l.front());
  auto it = ++l.iter(e + 3);
  EXPECT_EQ(*it, &insele);
  ++it;
  EXPECT_EQ(e + 4, *it);
}

TYPED_TEST_P(HListTest, ReplaceFirstElementWithNew)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  typename TypeParam::Elem repele;

  l.replace(l.front(), &repele);

  EXPECT_FALSE(l.in_list(ele));

  auto it = l.begin();
  EXPECT_EQ(*it, &repele);
  ++it;
  EXPECT_EQ(*it, ele + 1);
  ++it;
  EXPECT_EQ(*it, ele + 2);
}

TYPED_TEST_P(HListTest, ReplaceMiddleElementWithNew)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  typename TypeParam::Elem repele;

  l.replace(ele + 4, &repele);

  EXPECT_FALSE(l.in_list(ele + 4));

  auto it = ++l.iter(ele + 3);
  EXPECT_EQ(*it, &repele);
  ++it;
  EXPECT_EQ(*it, ele + 5);
  ++it;
  EXPECT_EQ(*it, ele + 6);
}

TYPED_TEST_P(HListTest, ReplaceEndElementWithNew)
{
  typename TypeParam::Elem ele[10];
  typename TypeParam::List l;

  for (int i = 9; i >= 0; --i)
    l.push_front(ele + i);

  typename TypeParam::Elem repele;

  l.replace(ele + 9, &repele);

  EXPECT_FALSE(l.in_list(ele + 9));

  auto it = ++l.iter(ele + 8);
  EXPECT_EQ(*it, &repele);
  ++it;
  EXPECT_EQ(it, l.end());
}



REGISTER_TYPED_TEST_CASE_P(HListTest,
                           InList, NotInListAfterPop,
                           InsertInEmpty, InsertAtBeginning,
                           InsertMiddle, InsertAtFinal, InsertAtEnd,
                           InsertAfterAtBeginning,
                           InsertAfterMiddle, InsertAfterAtFinal,
                           InsertBeforeInEmpty, InsertBeforeAtBeginning,
                           InsertBeforeMiddle, InsertBeforeAtFinal,
                           ReplaceFirstElementWithNew,
                           ReplaceMiddleElementWithNew,
                           ReplaceEndElementWithNew);

INSTANTIATE_TYPED_TEST_CASE_P(Hlist, HListTest, HlistTypes);
