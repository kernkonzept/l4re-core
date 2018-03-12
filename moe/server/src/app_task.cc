/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "app_task.h"
#include "globals.h"

using L4Re::Dataspace;


long
App_task::op_signal(L4Re::Parent::Rights, unsigned long sig, unsigned long val)
{
  switch (sig)
    {
    case 0: // exit
      {
        object_pool.cap_alloc()->free(obj_cap());
        if (val != 0)
          L4::cout << "MOE: task " << this << " exited with " << val
                   << '\n';

        return -L4_ENOREPLY;
      }
    default: break;
    }
  return L4_EOK;
}

App_task::App_task()
  : _task(L4::Cap<L4::Task>::Invalid),
    _thread(L4::Cap<L4::Thread>::Invalid),
    _alloc(Allocator::root_allocator()),
    _rm(_alloc->make_obj<Region_map>())
{
  auto c = object_pool.cap_alloc()->alloc(_rm.get());
  c->dec_refcnt(1);
}

App_task::~App_task()
{
  if (_rm)
    delete _rm.get();

  object_pool.cap_alloc()->free(_thread);
  object_pool.cap_alloc()->free(_task);
}
