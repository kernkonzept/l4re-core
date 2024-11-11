/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>

class Dispatcher
{
public:
  l4_msgtag_t dispatch(l4_msgtag_t tag, l4_umword_t obj, l4_utcb_t *);
};
