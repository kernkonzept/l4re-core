/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <l4/cxx/iostream>

/**
 * Tests that the cxx libc io backend implementation can be called and linked
 * from its associated library.
 */
int main(void)
{
  L4::iostream_init();
  L4::cerr << "Hello cerr from cxx_libc_io!\n";
  L4::cout << "Hello cout from cxx_libc_io!\n";

  return 0;
}
