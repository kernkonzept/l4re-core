// vim:set ft=cpp: -*- Mode: C++ -*-
/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Martin Decky <martin.decky@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Lock guard implementation.
 */

#pragma once

#include <pthread.h>
#include <stdlib.h>

namespace L4 {

/**
 * Basic lock guard implementation that prevents forgotten unlocks on exit
 * paths from a method or a block of code. Targeting #pthread_mutex_t.
 *
 * An instance of lock guard cannot be copied, but it can be moved.
 *
 * The typical usage pattern of the lock guard is:
 *
 * \code{.cpp}
 *   pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
 *
 *   {
 *     auto guard = L4Re::Lock_guard(mtx);
 *
 *     // Correctness check.
 *     assert(guard.status() == 0);
 *
 *     // Critical section protected by mtx.
 *
 *     // The mtx is automatically unlocked when guard goes out of scope.
 *   }
 * \endcode
 */
class Lock_guard
{
public:
  Lock_guard() = delete;
  Lock_guard(const Lock_guard &) = delete;
  Lock_guard &operator=(const Lock_guard &) = delete;

  /**
   * Construct the lock guard and lock the associated mutex.
   *
   * The error condition of the locking operation can be checked by the
   * #status() method.
   *
   * \param lock  Associated mutex to be locked.
   */
  explicit Lock_guard(pthread_mutex_t &lock) : _lock(&lock)
  {
    _status = pthread_mutex_lock(_lock);
  }

  /**
   * Move constructor from other lock guard.
   *
   * The mutex associated with the other lock guard is kept locked.
   *
   * \param guard  Lock guard to be moved.
   */
  Lock_guard(Lock_guard &&guard) : _lock(guard._lock), _status(guard._status)
  {
    guard.release();
  }

  /**
   * Move assignment from other lock guard.
   *
   * The mutex currently associated with this lock guard is unlocked. The mutex
   * associated with the other lock guard is kept locked.
   *
   * There is no mechanism for indicating any error conditions of the unlocking
   * operation. However, if the mutex has been previously locked successfully by
   * this class and if the implementation of the mutex behaves according to the
   * POSIX specification, the construction of this class guarantees that the
   * unlock operation does not fail.
   *
   * \param guard  Lock guard to be moved.
   */
  Lock_guard &operator=(Lock_guard &&guard)
  {
    // Unlock the currently associated mutex (if any).
    reset();

    // Move the state from the other guard.
    _lock = guard._lock;
    _status = guard._status;

    // Release the mutex from the other guard.
    guard.release();

    return *this;
  }

  /**
   * Return last lock/unlock operation error status.
   *
   * \return Zero indicating no errors, any other value indicating an error.
   */
  int status() const
  {
    return _status;
  }

  /**
   * Lock guard destructor.
   *
   * The associated mutex (if any) is unlocked.
   *
   * There is no mechanism for indicating any error conditions. However, if
   * the mutex has been previously locked successfully by this class and if the
   * implementation of the mutex behaves according to the POSIX specification,
   * the construction of this class guarantees that the unlock operation does
   * not fail.
   */
  ~Lock_guard()
  {
    reset();
  }

private:
  /**
   * Deassociate the mutex from the guard.
   *
   * Used internally to implement the move semantics only.
   */
  void release()
  {
    _lock = nullptr;
  }

  /**
   * Unlock and deassociate the mutex from the guard.
   *
   * Used internally to implement the move semantics and destruction only.
   *
   * The error condition of the unlocking operation can be checked by the
   * #status() method.
   */
  void reset()
  {
    // No mutex might be associated with this lock guard only if the mutex has
    // been moved to a different lock guard.
    if (_lock)
      {
        _status = pthread_mutex_unlock(_lock);
        release();
      }
  }

  pthread_mutex_t *_lock;
  int _status;
};

} // namespace L4
