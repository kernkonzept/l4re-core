/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <cstddef>
#include <l4/cxx/iostream>

void operator delete (void *obj) noexcept
{
  L4::cerr << "cxa pure delete operator called for object @"
           << L4::hex << obj << L4::dec << "\n";
}

#if __cplusplus >= 201400
void operator delete (void *obj, size_t size) noexcept;
void operator delete (void *obj, size_t size) noexcept
{
  L4::cerr << "cxa pure delete operator called for object @"
           << L4::hex << obj << L4::dec
           << " size " << size << "\n";
}
#endif
