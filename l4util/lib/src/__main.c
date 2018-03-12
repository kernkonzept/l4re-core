/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <stdlib.h>
#include <l4/crtn/crt0.h>
#include <l4/util/mbi_argv.h>

extern int main(int argc, char const *argv[]);

void
__main(int argc, char const *argv[])
{
  /* call constructors */
  crt0_construction();

  /* call main */
  exit(main(argc, argv));
}

