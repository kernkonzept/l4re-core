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

enum
{
  L4_fpage_cached   = L4_FPAGE_CACHEABLE << 4,
  L4_fpage_uncached = L4_FPAGE_UNCACHEABLE << 4,
  L4_fpage_buffered = L4_FPAGE_BUFFERABLE << 4,
};

struct Answer
{
private:
  void snd_base(unsigned long base)
  { l4_utcb_mr_u(utcb)->mr[0] = (base & (~0UL << 10)) | 8; }

public:
  l4_utcb_t *utcb;
  l4_msgtag_t tag;

  Answer(l4_utcb_t *utcb) : utcb(utcb), tag(l4_msgtag(0,0,0,0)) {}

  void error(int err) { tag = l4_msgtag(-err, 0, 0, 0); }

  void snd_fpage(l4_fpage_t const &fp, unsigned long snd_base = 0)
  {
    this->snd_base(snd_base);
    l4_utcb_mr_u(utcb)->mr[1] = fp.raw;
    tag = l4_msgtag(0, 0, 1, 0);
  }

  void snd_fpage(unsigned long addr, unsigned size, unsigned access,
                 bool cache)
  {
    snd_base(addr);
    if (cache)
      l4_utcb_mr_u(utcb)->mr[0] |= L4_fpage_cached;
    else
      l4_utcb_mr_u(utcb)->mr[0] |= L4_fpage_uncached;

    l4_utcb_mr_u(utcb)->mr[1] = l4_fpage(addr, size, access).raw;

    tag = l4_msgtag(0, 0, 1, 0);
  }

  bool failed() const
  { return tag.label() < 0; }

  void do_grant()
  { l4_utcb_mr_u(utcb)->mr[0] |= 2; }
};

