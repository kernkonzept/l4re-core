/**
 * \file
 * \brief   Constants
 */
/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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

