/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef SIGMA0_REGION_H__
#define SIGMA0_REGION_H__

#include <l4/cxx/iostream>
#include <l4/sys/types.h>

class Region
{
private:
  mutable unsigned long _l, _h;
  L4_fpage_rights _rights;

public:
  enum { Owner_mask = 0xfff };
  Region() : _l(0), _h(0), _rights(L4_FPAGE_RWX) {}
  Region(unsigned long start, unsigned long end, unsigned owner = 0 /*free*/,
         L4_fpage_rights rights = L4_FPAGE_RWX)
  : _l((start & ~Owner_mask) | owner), _h(end), _rights(rights)
  {
  }

  unsigned owner() const { return _l & Owner_mask; }
  L4_fpage_rights rights() const { return _rights; }
  unsigned long start() const { return _l & ~Owner_mask; }
  unsigned long end() const { return _h; }
  void restore_range_from(Region const &r) const
  {
    start(r.start());
    end(r.end());
  }

  void owner(unsigned owner) const { _l = (_l & ~Owner_mask) | owner; }
  void rights(L4_fpage_rights rights) { _rights = rights; }
  void start(unsigned long _start) const
  { _l = (_l & Owner_mask) | (_start & ~Owner_mask); }
  void end(unsigned long _end) const { _h = _end; }

  bool operator < (Region const &r) const { return end() < r.start(); }
  bool contains(Region const &r) const
  { return start() <= r.start() && end() >= r.end(); }

  bool operator == (Region const &r) const
  { return start() == r.start() && end() == r.end(); }

  bool valid() const { return end() >= start(); }

  static Region invalid() { return Region(~0UL, 0); }

  static Region bs(unsigned long start, unsigned long size, unsigned owner = 0,
                   L4_fpage_rights rights = L4_FPAGE_RWX)
  {
    if (size == 0)
      return invalid();

    return Region(start, start + size - 1, owner, rights);
  }

  static Region kr(unsigned long start, unsigned long end, unsigned owner = 0,
                   L4_fpage_rights rights = L4_FPAGE_RWX)
  { return bs(start, end - start, owner, rights); }

};

template< typename OS >
OS &operator << (OS &os, Region const &r)
{
  os << '[' << r.owner() << ':' << ((r.rights() & L4_FPAGE_RO) ? 'R' : '-')
     << ((r.rights() & L4_FPAGE_W) ? 'W' : '-')
     << ((r.rights() & L4_FPAGE_X) ? 'X' : '-') << ':' << (void *)r.start() << ';'
     << (void *)r.end() << ']';
  return os;
}
#endif
