/**
 * \file   l4re/lib/src/log.cc
 * \brief  Log
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
#include <l4/re/log>

#include <cstring>

namespace L4Re {

void
Log::printn(char const *string, int len) const noexcept
{
  l4_msg_regs_t store;
  l4_msg_regs_t *mr = l4_utcb_mr();

  memcpy(&store, mr, sizeof(store));

  while (len)
    {
      long l = write(string, len);
      if (l < 0)
        return;
      len -= l;
      string += l;
    }

  memcpy(mr, &store, sizeof(store));
}

void
Log::print(char const *string) const noexcept
{
  printn(string, __builtin_strlen(string));
}

}
