/*
 * (c) 2014 Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is licensed under the terms of the GNU Lesser General
 * Public License 2.1.
 * See the file COPYING-LGPL-2.1 for details.
 */
#pragma once

/**
 * \ingroup api_l4re_c
 * \brief Inhibitor C interface.
 */
#include <l4/sys/types.h>

EXTERN_C_BEGIN

/**
 * \brief Acquire an inhibitor lock.
 *
 * \param cap     Capability for the Inhibitor object (\see L4Re::Inhibitor)
 * \param id      ID of the inhibitor lock that shall be acquired.
 * \param reason  Reason why the inhibitor lock is acquired.
 *                (Used for informing the user or debugging.)
 * \return 0 for success, <0 on error
 * \see L4Re::Inhibitor::acquire()
 */
L4_CV long L4_EXPORT
l4re_inhibitor_acquire(l4_cap_idx_t cap, l4_umword_t id,
                       char const *reason);

/**
 * \brief Release an inhibitor lock.
 * \param cap  Capability for the Inhibitor object (\see L4Re::Inhibitor).
 * \param id   ID of inhibitor that shall be released.
 * \return 0 for success, <0 on error
 * \see L4Re::Inhibitor::release()
 */
L4_CV long L4_EXPORT
l4re_inhibitor_release(l4_cap_idx_t cap, l4_umword_t id);

/**
 * \brief Get information for the next available inhibitor lock.
 * \param cap  Capability to the Inhibitor object (\see L4Re::Inhibitor)
 * \param name a pointer to a buffer for the name of the lock.
 * \param len  the length of the available buffer (usually #Name_max is used)
 * \param current_id the ID of the last available lock, use -1 to get the
 *                   first lock.
 * \return the ID of the next available lock if there is one (in this case
 *         name shall contain the name of the inhibitor lock).
 * \return -L4_ENODEV if there are no more locks
 * \return any other negative failure value.
 * \see L4Re::Inhibitor::next_lock_info()
 */
L4_CV long L4_EXPORT
l4re_inhibitor_next_lock_info(l4_cap_idx_t cap, char *name,
                              unsigned len, l4_mword_t current_id);

EXTERN_C_END

