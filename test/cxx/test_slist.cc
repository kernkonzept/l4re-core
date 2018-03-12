/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/slist>

#include <l4/atkins/tap/main>

#include "bits_basic_list.h"

class Simple_elem : public cxx::S_list_item {};

struct SimpleSlist {
    typedef cxx::S_list<Simple_elem> List;
    typedef Simple_elem Elem;
};

typedef testing::Types<SimpleSlist> SlistTypes;

INSTANTIATE_TYPED_TEST_CASE_P(BasicSlist, BasicListTest, SlistTypes);
