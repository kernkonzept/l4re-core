/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/cxx/iostream>
#include "mem_man.h"
#include "mem_man_test.h"

#define ADD(a, b) \
  L4::cout << "add_free("#a","#b")\n"; \
  m.add_free(Region(a, b))

void mem_man_test()
{
  Mem_man m;
  L4::cout << "mem_man_test: ....\n";
  m.dump();
  ADD(0x0,      0x9000);
  ADD(0x15000, 0x20000);
  ADD(0x25000, 0x30000);
  ADD(0x35000, 0x40000);
  ADD(0x45000, 0x50000);
  m.dump();
  ADD(0x10000, 0x11000);
  m.dump();
  ADD(0x14000, 0x14000);
  m.dump();
  ADD(0x12000, 0x13000);
  m.dump();
  ADD(0x24000, 0x31000);
  m.dump();
  ADD(0x21000, 0x25000);
  m.dump();
  ADD(0x8000,  0x37000);
  m.dump();
  ADD(0x0,     0x44000);
  m.dump();
  L4::cout << "mem_man_test: done\n";
}
