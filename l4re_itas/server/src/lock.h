/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/re/unique_cap>
#include <l4/sys/semaphore>

/**
 * A reader-writer lock.
 *
 * The implementation relies just on kernel primitives. We cannot use pthread
 * primitives in ITAS because libpthread is not available.
 *
 * \note A reader-writer lock instance can serve up to 1023 threads.
 */
class Rw_lock
{
public:
  Rw_lock();

  Rw_lock(Rw_lock const &) = delete;
  Rw_lock &operator=(Rw_lock const &) = delete;

  void lock_read() noexcept;
  void unlock_read() noexcept;
  void lock_write() noexcept;
  void unlock_write() noexcept;

private:
  static constexpr unsigned Active_readers_mask   = 0x0000'03ffU;
  static constexpr unsigned Active_readers_inc    = 0x0000'0001U;
  static constexpr unsigned Blocked_readers_mask  = 0x000f'fc00U;
  static constexpr unsigned Blocked_readers_inc   = 0x0000'0400U;
  static constexpr unsigned Blocked_readers_shift = 10;
  static constexpr unsigned Writers_mask          = 0x3ff0'0000U;
  static constexpr unsigned Writers_inc           = 0x0010'0000U;

  L4Re::Unique_cap<L4::Semaphore> _rd_sem;
  L4Re::Unique_cap<L4::Semaphore> _wr_sem;
  unsigned _status = 0;
};

class Rw_lock_read_scope
{
  Rw_lock &_lock;

public:
  Rw_lock_read_scope(Rw_lock &lock) : _lock(lock) { lock.lock_read(); }
  ~Rw_lock_read_scope() { _lock.unlock_read(); }

  Rw_lock_read_scope(const Rw_lock_read_scope &) = delete;
  Rw_lock_read_scope(Rw_lock_read_scope &&) = delete;
  Rw_lock_read_scope &operator=(const Rw_lock_read_scope &) = delete;
  Rw_lock_read_scope &operator=(Rw_lock_read_scope &&) = delete;
};

class Rw_lock_write_scope
{
  Rw_lock &_lock;

public:
  Rw_lock_write_scope(Rw_lock &lock) : _lock(lock) { lock.lock_write(); }
  ~Rw_lock_write_scope() { _lock.unlock_write(); }

  Rw_lock_write_scope(const Rw_lock_write_scope &) = delete;
  Rw_lock_write_scope(Rw_lock_write_scope &&) = delete;
  Rw_lock_write_scope &operator=(const Rw_lock_write_scope &) = delete;
  Rw_lock_write_scope &operator=(Rw_lock_write_scope &&) = delete;
};
