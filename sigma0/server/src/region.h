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

class Region
{
private:
  mutable unsigned long _l, _h;
public:
  enum { Owner_mask = 0xfff };
  Region() : _l(0), _h(0) {}
  Region(unsigned long start, unsigned long end, unsigned owner = 0 /*free*/)
    : _l((start & ~Owner_mask) | owner), _h(end) {}

  unsigned owner() const { return _l & Owner_mask; }
  unsigned long start() const { return _l & ~Owner_mask; }
  unsigned long end() const { return _h; }

  void owner(unsigned owner) const { _l = (_l & ~Owner_mask) | owner; }
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

  static Region bs(unsigned long start, unsigned long size, unsigned owner = 0)
  {
    if (size == 0)
      return invalid();

    return Region(start, start + size - 1, owner);
  }

  static Region kr(unsigned long start, unsigned long end, unsigned owner = 0)
  { return bs(start, end - start, owner); }

};


template< typename OS >
OS &operator << (OS &os, Region const &r)
{
  os << '[' << r.owner() << ':' << (void *)r.start() << ';' << (void *)r.end() << ']';
  return os;
}
#endif

