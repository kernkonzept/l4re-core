/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/cxx/iostream>

extern "C" void __cxa_pure_virtual(void);

void __cxa_pure_virtual()
{
  L4::cerr << "cxa pure virtual function called\n";
}


extern "C" void __pure_virtual(void);

void __pure_virtual()
{
  L4::cerr << "cxa pure virtual function called\n";
}
