/*
 * Copyright (C) 2008 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef BID_VARIANT_FLAG_NOFPU
int getloadavg(double loadavg[], int nelem);
int getloadavg(double loadavg[], int nelem)
{
  (void)nelem;
  loadavg[0] = 0.7;
  loadavg[1] = 0.7;
  loadavg[2] = 0.7;
  return 3;
}
#endif
