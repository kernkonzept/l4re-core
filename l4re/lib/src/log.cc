/**
 * \file
 * \brief  Log
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/log>

#include <string.h>

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
