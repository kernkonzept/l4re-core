/**
 * \file
 * \brief   Constants
 */
/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
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
#pragma once

#include <l4/sys/consts.h>

enum
{
  L4RE_THIS_TASK_CAP = 1UL << L4_CAP_SHIFT,
};

/**
 * Defaults for local thread priorities
 *
 * Priorities are to be seen as local. These are used by the loader and
 * libpthread. The are to be understood as 'local', which means the actual
 * priority of the thread (as seen by the kernel) is the base priority as
 * defined by the scheduler plus the local priority.
 */
enum
{
  L4RE_MAIN_THREAD_PRIO = 2, /* Priority of the main thread */
};

