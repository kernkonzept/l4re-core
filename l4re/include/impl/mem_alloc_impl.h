/**
 * \file
 * \brief  Memory allocator client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#include <l4/re/mem_alloc>
#include <l4/re/mem_alloc-sys.h>
#include <l4/re/dataspace>
#include <l4/re/error_helper>

#include <l4/sys/factory>


namespace L4Re
{

long
Mem_alloc::alloc(long size,
                 L4::Cap<Dataspace> mem, unsigned long flags,
                 unsigned long align) const throw()
{
  L4::Cap<L4::Factory> f(cap());
  return l4_error(f->create(mem, L4Re::Dataspace::Protocol)
                  << l4_mword_t(size)
                  << l4_umword_t(flags)
                  << l4_umword_t(align));
}

long
Mem_alloc::free(L4::Cap<Dataspace>) const throw()
{
  return 0;
}

};
