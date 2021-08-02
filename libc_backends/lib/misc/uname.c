/*
 * Copyright (C) 2009, 2010 TU Dresden.
 *               2015 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *            Alexander Warg <alexander.warg@kernkonzept.com>
 *            Björn Döbel <doebel@os.inf.tu-dresden.de>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <sys/utsname.h>
#include <string.h>

int uname(struct utsname *u)
{
  strncpy(u->sysname, "L4Re", sizeof(u->sysname));
  u->sysname[sizeof(u->sysname) - 1] = 0;

  strncpy(u->nodename, "localhost", sizeof(u->nodename));
  u->nodename[sizeof(u->nodename) - 1] = 0;

  strncpy(u->release, "L4Re", sizeof(u->release));
  u->release[sizeof(u->release) - 1] = 0;

  strncpy(u->version, "0.x", sizeof(u->version));
  u->version[sizeof(u->version) - 1] = 0;

#ifdef ARCH_arm
  strncpy(u->machine, "arm", sizeof(u->machine));
#elif defined(ARCH_arm64)
  strncpy(u->machine, "aarch64", sizeof(u->machine));
#elif defined(ARCH_x86)
  strncpy(u->machine, "i686", sizeof(u->machine));
#elif defined(ARCH_amd64)
  strncpy(u->machine, "x86_64", sizeof(u->machine));
#elif defined(ARCH_ppc32)
  strncpy(u->machine, "ppc32", sizeof(u->machine));
#elif defined(ARCH_sparc)
  strncpy(u->machine, "sparcv8", sizeof(u->machine));
#elif defined(ARCH_mips)
  strncpy(u->machine, "mips", sizeof(u->machine));
#elif defined(ARCH_riscv)
  strncpy(u->machine, "riscv", sizeof(u->machine));
#else
#error Add your arch.
#endif
  u->machine[sizeof(u->machine) - 1] = 0;

  return 0;
}
