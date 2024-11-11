/**
 * \file
 * \brief Backtrace
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <sys/cdefs.h>

__BEGIN_DECLS

/**
 * \brief Fill backtrace structure.
 *
 * \param pc_array  Array of instruction pointers.
 * \param max_len   Length of array.
 * \return Number of entries
 */
int l4util_backtrace(void **pc_array, int max_len);

__END_DECLS
