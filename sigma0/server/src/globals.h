/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef GLOBALS_H
#define GLOBALS_H

#include <l4/sys/types.h>

/* Special options for compatibility reasons */

enum {
  debug_errors        = 1,
  debug_warnings      = 0,
  debug_ipc           = 0,
  debug_memory_maps   = 1,

};

/* globals defined here (when included from globals.c) */
#define PROG_NAME "SIGMA0"

enum {
  sigma0_taskno = 2,
  root_taskno = 4
};

extern "C" void L4_NORETURN _exit(int);

inline void L4_NORETURN abort()
{
  _exit(1);
}

#endif
