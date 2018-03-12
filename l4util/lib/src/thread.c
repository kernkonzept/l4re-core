/**
 * \file
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Michael Hohmuth <hohmuth@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/types.h>
#include <l4/sys/factory.h>
#include <l4/sys/thread.h>
#include <l4/sys/scheduler.h>

#include <l4/util/thread.h>

L4_CV long
l4util_create_thread(l4_cap_idx_t id, l4_utcb_t *thread_utcb,
                     l4_cap_idx_t factory,
                     l4_umword_t pc, l4_umword_t sp, l4_cap_idx_t pager,
                     l4_cap_idx_t task,
                     l4_cap_idx_t scheduler, l4_sched_param_t scp) L4_NOTHROW
{
  l4_msgtag_t res = l4_factory_create_thread(factory, id);
  if (l4_error(res))
    return l4_error(res);

  l4_thread_control_start();
  l4_thread_control_pager(pager);
  l4_thread_control_bind(thread_utcb, task);
  res = l4_thread_control_commit(id);
  if (l4_error(res))
    return l4_error(res);

  res = l4_thread_ex_regs(id, pc, sp, 0);
  if (l4_error(res))
    return l4_error(res);

  if (!l4_is_invalid_cap(scheduler))
    {
      res = l4_scheduler_run_thread(scheduler, id, &scp);
      if (l4_error(res))
        return l4_error(res);
    }

  return 0;
}
