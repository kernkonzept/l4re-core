/**
 * \file
 * \brief  settimeofday implementation
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <sys/time.h>
#include <errno.h>

// do nothing for now
int settimeofday(const struct timeval *tv , const struct timezone *tz)
{
  (void)tv; (void)tz;
  errno = EPERM;
  return -1;
}
