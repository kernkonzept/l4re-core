/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <vector>
#include <algorithm>

#include <l4/cxx/avl_tree>
#include <l4/cxx/ref_ptr>

#include <l4/atkins/tap/main>

struct Int_node : public cxx::Avl_tree_node, cxx::Ref_obj
{
  typedef int Key_type;

  static int key_of(Int_node const *node) { return node->value; }

  int value;

  Int_node(int v) : value(v) {}
};

typedef cxx::Avl_tree<Int_node, Int_node> Int_tree;

struct IntAvlTree : public ::testing::Test
{
  std::vector<cxx::Ref_ptr<Int_node>> nodes;
  Int_tree tree;

  /// Allocate a new node that will be auto-destroyed at the end of the test.
  Int_node *node(int value)
  {
    cxx::Ref_ptr<Int_node> p = cxx::make_ref_obj<Int_node>(value);
    // record the node as inserted
    nodes.push_back(p);

    return p.get();
  }
};

TEST_F(IntAvlTree, IteratorsOnEmpty)
{
  EXPECT_EQ(tree.begin(), tree.end());
  EXPECT_EQ(tree.rbegin(), tree.rend());
}

TEST_F(IntAvlTree, InsertRemove)
{
  EXPECT_EQ(tree.begin(), tree.end());

  EXPECT_TRUE(tree.insert(node(50)).second);
  EXPECT_TRUE(tree.insert(node(60)).second);
  EXPECT_TRUE(tree.insert(node(70)).second);
  EXPECT_TRUE(tree.insert(node(80)).second);
  EXPECT_TRUE(tree.insert(node(81)).second);
  EXPECT_TRUE(tree.insert(node(45)).second);
  EXPECT_TRUE(tree.insert(node(40)).second);
  EXPECT_TRUE(tree.insert(node(35)).second);
  EXPECT_TRUE(tree.insert(node(33)).second);
  EXPECT_TRUE(tree.insert(node(32)).second);
  EXPECT_TRUE(tree.insert(node(41)).second);
  EXPECT_TRUE(tree.insert(node(42)).second);
  EXPECT_TRUE(tree.insert(node(43)).second);
  EXPECT_TRUE(tree.insert(node(31)).second);
  EXPECT_TRUE(tree.insert(node(30)).second);
  EXPECT_TRUE(tree.insert(node(29)).second);
  EXPECT_TRUE(tree.insert(node(28)).second);
  EXPECT_TRUE(tree.insert(node(27)).second);
  EXPECT_TRUE(tree.insert(node(26)).second);

  // and check that all are in
  for (auto &n : nodes)
    EXPECT_EQ(n.get(), tree.find_node(n->value));

  // remove some
  EXPECT_EQ(29, tree.remove(29)->value);
  EXPECT_EQ(70, tree.remove(70)->value);
  EXPECT_EQ(50, tree.remove(50)->value);
  EXPECT_EQ(60, tree.remove(60)->value);
  EXPECT_EQ(26, tree.remove(26)->value);
  EXPECT_EQ(32, tree.remove(32)->value);
  EXPECT_EQ(35, tree.remove(35)->value);
  EXPECT_EQ(30, tree.remove(30)->value);
  EXPECT_EQ(45, tree.remove(45)->value);
  EXPECT_EQ(43, tree.remove(43)->value);
  EXPECT_EQ(80, tree.remove(80)->value);
  EXPECT_EQ(41, tree.remove(41)->value);
  EXPECT_EQ(42, tree.remove(42)->value);
}

TEST_F(IntAvlTree, InsertDuplicate)
{
  EXPECT_EQ(tree.begin(), tree.end());

  Int_node *n = node(34);
  Int_node *n2 = node(34);

  auto r = tree.insert(n);

  EXPECT_EQ(r.first, n);
  EXPECT_EQ(r.second, true);

  r = tree.insert(n2);

  EXPECT_EQ(r.first, n);
  EXPECT_EQ(r.second, false);

  r = tree.insert(n);

  EXPECT_EQ(r.first, n);
  EXPECT_EQ(r.second, false);
}

TEST_F(IntAvlTree, RemoveDuplicate)
{
  EXPECT_TRUE(tree.insert(node(555)).second);

  EXPECT_EQ(nullptr, tree.remove(-1));
  EXPECT_EQ(nullptr, tree.remove(554));
  EXPECT_EQ(nullptr, tree.remove(556));
  EXPECT_EQ(nullptr, tree.remove(1045));
  EXPECT_EQ(nullptr, tree.remove(0));

  EXPECT_EQ(555, tree.remove(555)->value);
  EXPECT_EQ(nullptr, tree.remove(555));
}

TEST_F(IntAvlTree, RemoveAll)
{
  std::vector<int> ids { 6, 9, 0, 1, 4 };

  for (int i : ids)
    EXPECT_TRUE(tree.insert(node(i)).second);

  for (int i : ids)
    EXPECT_EQ(i, tree.remove(i)->value);

  EXPECT_EQ(tree.begin(), tree.end());
}

TEST_F(IntAvlTree, IteratorOrder)
{
  std::vector<int> ids { 1, 5, 6, 100, 102, 103, 999 };

  // create nodes in order
  for (int i : ids)
    node(i);

  // and insert them in random order
  std::random_shuffle(nodes.begin(), nodes.end());
  for (auto &n : nodes)
    tree.insert(n.get());

  // iteration should return them in natural order
  auto it = tree.begin();
  for (int i: ids)
    {
      EXPECT_EQ(i, it->value);
      ++it;
    }
  EXPECT_EQ(it, tree.end());
}

TEST_F(IntAvlTree, ConstIteratorOrder)
{
  std::vector<int> ids { 1, 5, 6, 100, 102, 103, 999 };

  // create nodes in order
  for (int i : ids)
    node(i);

  // and insert them in random order
  std::random_shuffle(nodes.begin(), nodes.end());
  for (auto &n : nodes)
    tree.insert(n.get());

  // iteration should return them in natural order
  Int_tree::Const_iterator it = tree.begin();
  for (int i: ids)
    {
      EXPECT_EQ(i, it->value);
      ++it;
    }
  EXPECT_EQ(it, tree.end());
}

TEST_F(IntAvlTree, ReverseIteratorOrder)
{
  std::vector<int> ids { -45, 0, 2, 3, 10008, 80001, 80002 };

  // create nodes in order
  for (int i : ids)
    node(i);

  // and insert them in random order
  std::random_shuffle(nodes.begin(), nodes.end());
  for (auto &n : nodes)
    tree.insert(n.get());

  // reverse iteration should return them in reverse natural order
  auto it = tree.rbegin();
  for (auto id = ids.rbegin(); id != ids.rend(); ++id)
    {
      EXPECT_EQ(*id, it->value);
      ++it;
    }
  EXPECT_EQ(it, tree.rend());
}

TEST_F(IntAvlTree, ReverseConstIteratorOrder)
{
  std::vector<int> ids { -45, 0, 2, 3, 10008, 80001, 80002 };

  // create nodes in order
  for (int i : ids)
    node(i);

  // and insert them in random order
  std::random_shuffle(nodes.begin(), nodes.end());
  for (auto &n : nodes)
    tree.insert(n.get());

  // reverse iteration should return them in reverse natural order
  Int_tree::Const_rev_iterator it = tree.rbegin();
  for (auto id = ids.rbegin(); id != ids.rend(); ++id)
    {
      EXPECT_EQ(*id, it->value);
      ++it;
    }
  EXPECT_EQ(it, tree.rend());
}

TEST_F(IntAvlTree, LowerBoundNode)
{
  std::vector<int> ids { 99999, -1, 985, 3, 1, -88, 0, -5 };

  for (int i : ids)
    EXPECT_TRUE(tree.insert(node(i)).second);

  EXPECT_EQ(-88, tree.lower_bound_node(-9999999)->value);
  EXPECT_EQ(-88, tree.lower_bound_node(-88)->value);
  EXPECT_EQ(-5, tree.lower_bound_node(-6)->value);
  EXPECT_EQ(-1, tree.lower_bound_node(-1)->value);
  EXPECT_EQ(0, tree.lower_bound_node(0)->value);
  EXPECT_EQ(1, tree.lower_bound_node(1)->value);
  EXPECT_EQ(985, tree.lower_bound_node(100)->value);
  EXPECT_EQ(99999, tree.lower_bound_node(99999)->value);
  EXPECT_EQ(nullptr, tree.lower_bound_node(999991));
}

TEST_F(IntAvlTree, LowerBoundNodeOnEmptyTree)
{
  EXPECT_EQ(nullptr, tree.lower_bound_node(0));
}

TEST_F(IntAvlTree, LowerBoundNodeOnSingleNodeTree)
{
  tree.insert(node(1));

  EXPECT_EQ(1, tree.lower_bound_node(0)->value);
  EXPECT_EQ(nullptr, tree.lower_bound_node(2));
}
