/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#pragma once

#include <time.h>
#include <inttypes.h>

extern uint64_t __libc_l4_rt_clock_offset;

__BEGIN_DECLS

int libc_backend_rt_clock_gettime(struct timespec *tp);

__END_DECLS
