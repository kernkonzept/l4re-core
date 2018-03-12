/*
 * (c) 2013 Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <stdlib.h>
#include <cstdio>
#include <unistd.h>

static void set_initial_cwd(void) __attribute__((constructor));
static void set_initial_cwd()
{
  char *initial_dir = getenv("INIT_CWD");
  if (initial_dir)
    {
      int err = chdir(initial_dir);
      if (err != 0)
        printf("libinitcwd: could not chdir to '%s'.\n", initial_dir);
    }
}

