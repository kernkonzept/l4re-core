/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#pragma once

#include <map>

#include <gtest/gtest.h>

class Test_track_alloc : public testing::Test
{
private:
  static std::map<void *, size_t> allocated;
  static int bad_allocs;

public:
  virtual void SetUp()
  {
    allocated.clear();
    bad_allocs = 0;
  }

  virtual void TearDown()
  {
    EXPECT_TRUE(allocated.empty());
    EXPECT_EQ(0, bad_allocs);
  }

  static void add_allocation(void *p, size_t sz)
  { allocated[p] = sz;}

  static void remove_allocation(void *p, size_t sz)
  {
    auto it = allocated.find(p);
    if (it == allocated.end() || it->second != sz)
      bad_allocs++;
    else
      allocated.erase(it);
  }
};

std::map<void *, size_t> Test_track_alloc::allocated;
int Test_track_alloc::bad_allocs;

template <typename T>
struct TrackingAlloc
{
  enum { can_free = true };

  static T *alloc() throw()
  {
    void *p = malloc(sizeof(T));

    if (p)
      Test_track_alloc::add_allocation(p, sizeof(T));

    return (T *) p;
  }

  static void free(T *t) throw()
  {
    Test_track_alloc::remove_allocation(t, sizeof(T));
    std::free(t);
  }
};
