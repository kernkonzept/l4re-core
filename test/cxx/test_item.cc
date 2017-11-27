/*
 * Copyright (C) 2017 Kernkonzept GmbH.
 * Author(s): Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/atkins/tap/main>
#include <gtest/gtest-spi.h>

#include "item"

/**
 * Behavior test for 'class Item'.
 */
using namespace Container_test;

/**
 * Item's constructor adds the address of the new item to the item address set.
 * Item's destructor removes the address from said set again.
 */
TEST(TestItem, CtorDtor)
{
    {
      Item ite;
      EXPECT_EQ(1U, Item::item_address.size())
        << "One item address was stored.";
    }
  ASSERT_TRUE(Item::item_address.empty())
    << "The item's destructor removed its address from the set.";
}

/**
 * Item's copy-constructor adds the address of the new item to the item address
 * set. The destructor of the copied item removes its address correctly.
 */
TEST(TestItem, CopyCtorDtor)
{
    {
      Item ite;
      Item ite2(ite);
      EXPECT_EQ(2U, Item::item_address.size())
        << "Two items inserted their addresses.";

      auto it = Item::item_address.begin();
      EXPECT_EQ(**it, **(++it)) << "The two items are copies of each other.";
      EXPECT_EQ(Item::item_address.end(), ++it)
        << "The set contains exactly two items.";
    }
  ASSERT_TRUE(Item::item_address.empty())
    << "Both item's destructors removed themselves from the address set.";
}

/**
 * Item's copy-assignment copies the ID and does not change the address set.
 */
TEST(TestItem, CopyAssignment)
{
  Item ite;
  Item ite2;
  EXPECT_NE(ite.id(), ite2.id()) << "The two items have different IDs.";

  ite2 = ite;

  EXPECT_EQ(2U, Item::item_address.size())
    << "Both items are in the address set.";
  EXPECT_EQ(ite.id(), ite2.id()) << "Both items have the same ID.";
}
/**
 * Item's move-constructor invokes the destructor of the old item.
 */
TEST(TestItem, MoveCtorDtor)
{
  Item *ite_addr;

    {
      Item ite;
      ite_addr = &ite;
      unsigned ite_id = ite.id();
      Item ite2(std::move(ite));

      EXPECT_EQ(1U, Item::item_address.count(&ite))
        << "Moved item address is still in the address set.";
      EXPECT_EQ(ite_id, ite2.id()) << "Moved-to item has the same ID.";
      EXPECT_EQ(Item::Invalid_id, ite.id())
        << "The item ID indicates that the content was moved.";
    }

  EXPECT_EQ(0U, Item::item_address.count(ite_addr))
    << "Moved item object was destroyed and removed from the item address set.";
}

/**
 * Item's move-assignment moves the ID to the new instance, zeros the ID field
 * of the old instance and removes the old instance from the address set.
 */
TEST(TestItem, MoveAssignment)
{
  Item ite;
  unsigned ite_id = ite.id();
  Item ite2;
  EXPECT_NE(ite_id, ite2.id()) << "The two items have different IDs.";

  ite2 = std::move(ite);

  EXPECT_EQ(2U, Item::item_address.size())
    << "The move-assignment inserted the new item into the address set.";
  EXPECT_EQ(ite_id, ite2.id())
    << "The move-to item has the ID of the move-from item.";
  EXPECT_EQ(Item::Invalid_id, ite.id())
    << "The item ID indicates that the content was moved.";
}

/**
 * Item's move-constructor records an error in case the moved-from content is
 * invalid.
 *
 * \note The EXPECT statement is located in 'class Item'.
 *
 * \see gTest AdvanceGuide - Catching Failures
 */
TEST(TestItem, MoveCtorFromInvalid)
{
  Item ite;
  unsigned ite_id = ite.id();
  Item ite2(std::move(ite));

  // ite3 is not defined outside this macro.
  EXPECT_NONFATAL_FAILURE(
    Item ite3(std::move(ite)), // Erroneous move-constructor
    "The object to move-from has valid content.");

  EXPECT_EQ(ite_id, ite2.id())
    << "The move-constructor assigns the move-from item ID to the move-to "
       "item instance.";
  EXPECT_EQ(Item::Invalid_id, ite.id())
    << "The move-from item ID is invalid after the move.";
}

/**
 * Item's move-assignment operator records an error in case the moved-from
 * content is invalid.
 *
 * \note The EXPECT statement is located in 'class Item'.
 *
 * \see gTest AdvanceGuide - Catching Failures
 */
TEST(TestItem, MoveAssignmentFromInvalid)
{
  Item ite;
  Item ite2;
  unsigned ite_id = ite.id();
  ite2 = std::move(ite);
  unsigned ite2_id_post_move = ite2.id();

  EXPECT_EQ(ite_id, ite2_id_post_move)
    << "The move-to item has the ID of the move-from item.";
  EXPECT_EQ(Item::Invalid_id, ite.id()) << "The move-from item ID is invalid.";

  EXPECT_NONFATAL_FAILURE(
    ite2 = std::move(ite), // Erroneous move assignment
    "The object to move-from has valid content.");

  EXPECT_EQ(Item::Invalid_id, ite2.id())
    << "The move-assignment succeeds and assigns the invalid ID in the "
       "move-from item to the move-to item.";
}

/**
 * Item's copy-constructor records an error in case the copy-from content is
 * invalid.
 *
 * \note The EXPECT statement is located in 'class Item'.
 *
 * \see gTest AdvanceGuide - Catching Failures
 */
TEST(TestItem, CopyCtorFromInvalid)
{
  Item ite;
  Item ite2(std::move(ite));

  EXPECT_NONFATAL_FAILURE(Item ite3(ite), // Erroneous copy-constructor
                          "The object to copy from has valid content.");
}

/**
 * Item's copy-assignment operator records an error in case the copy-from
 * content is invalid.
 *
 * \note The EXPECT statement is located in 'class Item'.
 *
 * \see gTest AdvanceGuide - Catching Failures
 */
TEST(TestItem, CopyAssignmentFromInvalid)
{
  Item ite;
  Item ite2(std::move(ite));
  Item ite3;

  // ite3 is not defined outside this macro.
  EXPECT_NONFATAL_FAILURE(ite3 = ite, // Erroneous copy assignment
                          "The object to copy-assign from has valid content.");
  EXPECT_EQ(Item::Invalid_id, ite3.id());
}
