/**
 * \file
 * \brief List of all init priorities.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#pragma once

#define INIT_PRIO_EARLY                     101
#define INIT_PRIO_L4RE_UTIL_CAP_ALLOC       200
#define INIT_PRIO_VFS_INIT                  400
#define INIT_PRIO_LIBIO_INIT                1100
#define INIT_PRIO_RTC_L4LIBC_INIT           1200
#define INIT_PRIO_LATE                      5000


