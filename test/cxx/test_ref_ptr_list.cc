/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/cxx/ref_ptr_list>

#include <l4/atkins/tap/main>

#include "bits_smart_ptr_list.h"

static int last_destructed = -1;

struct Simple_obj_elem : public cxx::Ref_obj_list_item<Simple_obj_elem>
{
  explicit Simple_obj_elem(int ident) : id(ident) {}

  virtual ~Simple_obj_elem()
  {
    last_destructed = id;
  }

  int id;
};

struct Simple_ref_elem
: public cxx::Ref_ptr_list_item<Simple_ref_elem>,
  public cxx::Ref_obj
{
  explicit Simple_ref_elem(int ident) : id(ident) {}

  virtual ~Simple_ref_elem()
  {
    last_destructed = id;
  }

  int id;
};


template <typename ELEM>
struct SimpleRefList
{
  using Elem = ELEM;
  using List = cxx::Ref_ptr_list<Elem>;

  static cxx::Ref_ptr<Elem> make_elem(int id = 0)
  { return cxx::make_ref_obj<Elem>(id); }

};

typedef testing::Types<SimpleRefList<Simple_ref_elem>,
                       SimpleRefList<Simple_obj_elem> > ListTypes;

INSTANTIATE_TYPED_TEST_CASE_P(Basic, BasicSmartListTest, ListTypes);

/**
 * Allocation tests for Ref_ptr_lists.
 */
struct RefListTest : public testing::Test
{
  void SetUp()
  {
    last_destructed = -1;
  }

  static cxx::Ref_ptr<Simple_obj_elem> make_elem(int id = 0)
  { return cxx::make_ref_obj<Simple_obj_elem>(id); }

  cxx::Ref_ptr_list<Simple_obj_elem> l;
};

/**
 * Push_front() moves the element into the list without destroying it.
 */
TEST_F(RefListTest, ElementNotDestructedAfterPushFront)
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
 * Push_front() may just make an additional reference of the element.
 */
TEST_F(RefListTest, PushFrontAddsRef)
{
  auto e = make_elem(56465);
  l.push_front(e);
  EXPECT_TRUE(!!e)
    << "The original copy of the element is still valid after push_front().";
  l.push_front(cxx::move(e));
  EXPECT_FALSE(!!e)
    << "The original copy of the element is not valid after moving the element.";
}

/**
 * Push_back() may just make an additional reference of the element.
 */
TEST_F(RefListTest, PushBackAddsRef)
{
  auto e = make_elem(56465);
  l.push_back(e);
  EXPECT_TRUE(!!e)
    << "The original copy of the element is still valid after push_front().";
  l.push_back(cxx::move(e));
  EXPECT_FALSE(!!e)
    << "The original copy of the element is not valid after moving the element.";
}

/**
 * Push_back() moves the element into the list without destroying it.
 */
TEST_F(RefListTest, ElementNotDestructedAfterPushBack)
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
TEST_F(RefListTest, ElementCanBeDestructedAfterPopFront)
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
TEST(RefListDestructionTest, ElementIsDestructedWithList)
{
  last_destructed = -1;
  {
    cxx::Ref_ptr_list<Simple_obj_elem> l;
    l.push_back(cxx::make_ref_obj<Simple_obj_elem>(6678));
    EXPECT_EQ(-1, last_destructed)
      << "The element is not destructed after inserting to the list.";
  }

  EXPECT_EQ(6678, last_destructed)
    << "The element in the list is destructed after list goes out of scope.";
}
