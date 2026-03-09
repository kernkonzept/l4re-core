/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/env>
#include <l4/re/error_helper>

#include "lock.h"
#include "globals.h"

Rw_lock::Rw_lock()
{
  _rd_sem = L4Re::make_unique_cap<L4::Semaphore>(Global::cap_alloc);
  L4Re::chkcap(_rd_sem.get(), "Rw_lock: alloc _rd_sem");
  L4Re::chksys(L4Re::Env::env()->factory()->create(_rd_sem.get()),
               "Rw_lock: create _rd_sem");

  _wr_sem = L4Re::make_unique_cap<L4::Semaphore>(Global::cap_alloc);
  L4Re::chkcap(_wr_sem.get(), "Rw_lock: alloc _wr_sem");
  L4Re::chksys(L4Re::Env::env()->factory()->create(_wr_sem.get()),
               "Rw_lock: create _wr_sem");
}

void Rw_lock::lock_read() noexcept
{
  bool sleep;

  do
    {
      unsigned status = __atomic_load_n(&_status, __ATOMIC_RELAXED);
      unsigned new_status;
      do
        {
          new_status = status;
          if (L4_UNLIKELY(status & Writers_mask))
            {
              // There is at least one writer. We'll queue us in the _rd_sem
              // from where the (last) writer will wake us upon leaving.
              new_status += Blocked_readers_inc;
              sleep = true;
            }
          else
            {
              // No writer. Add myself as active reader.
              new_status += Active_readers_inc;
              sleep = false;
            }
        }
      while (!__atomic_compare_exchange_n(&_status, &status, new_status, true,
                                          __ATOMIC_ACQUIRE, __ATOMIC_RELAXED));

      if (L4_UNLIKELY(sleep))
        _rd_sem->down();
    }
  while (sleep);
}

void Rw_lock::unlock_read() noexcept
{
  unsigned status = __atomic_sub_fetch(&_status, Active_readers_inc, __ATOMIC_RELEASE);

  // Release one waiting writer if we were the last active reader.
  if (L4_UNLIKELY((status & Active_readers_mask) == 0 && (status & Writers_mask) != 0))
    _wr_sem->up();
}

void Rw_lock::lock_write() noexcept
{
  unsigned status = __atomic_fetch_add(&_status, Writers_inc, __ATOMIC_ACQUIRE);

  // Sleep if there were any readers or writers
  if (status != 0)
    _wr_sem->down();
}

void Rw_lock::unlock_write() noexcept
{
  unsigned status = __atomic_load_n(&_status, __ATOMIC_RELAXED);
  unsigned new_status;
  do
    {
      new_status = status - Writers_inc;
      // If we were the last writer, we are the one that releases all blocked
      // readers.
      if ((new_status & Writers_mask) == 0)
        new_status &= ~Blocked_readers_mask;
    }
  while (!__atomic_compare_exchange_n(&_status, &status, new_status, true,
                                      __ATOMIC_RELEASE, __ATOMIC_RELAXED));

  if (new_status & Writers_mask)
    // Writers are waiting. Release one...
    _wr_sem->up();
  else
    // If we were the last writer, release the readers...
    for (unsigned i = (status & Blocked_readers_mask) >> Blocked_readers_shift;
         i > 0;
         --i)
      _rd_sem->up();
}
