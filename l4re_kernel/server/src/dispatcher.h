/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>

class Dispatcher
{
public:
  l4_msgtag_t dispatch(l4_msgtag_t tag, l4_umword_t obj, l4_utcb_t *);
};
