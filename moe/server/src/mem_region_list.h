/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/cxx/string>
#include <l4/sys/l4int.h>

#include <cassert>
#include <cstddef>

namespace Moe
{

struct Mem_range
{
  l4_addr_t start;
  l4_addr_t end; // inclusive

  constexpr Mem_range() : start(static_cast<l4_addr_t>(-1)), end(0) {}
  constexpr Mem_range(l4_addr_t start, l4_addr_t end) : start(start), end(end)
  { assert(valid()); }

  constexpr bool valid() const
  { return start <= end; }

  /**
   * Calculate size of region.
   *
   * \note Overflows to zero if region spans the entire address space.
   */
  size_t size() const
  { return end - start + size_t{1}; }

  /** Compare two regions. */
  friend bool operator < (Mem_range const &lhs, Mem_range const &rhs)
  { return lhs.end < rhs.start; }

  /** Check for an overlap. */
  bool overlaps(Mem_range const &o) const
  { return !(*this < o) && !(o < *this); }

  /** Test if o is a sub-region of ourselves. */
  bool contains(Mem_range const &o) const
  { return start <= o.start && end >= o.end; }

  /** Check for adjacent regions. */
  bool adjacent(Mem_range const &o) const
  {
    return (end + 1 > end && (end + 1) == o.start)
           || (o.end + 1 > o.end && start == (o.end + 1));
  }

  void merge(Mem_range const &r)
  {
    assert(overlaps(r) || adjacent(r));

    start = start < r.start ? start : r.start;
    end = end > r.end ? end : r.end;
  }

  /** Calculate the intersection. */
  Mem_range intersect(Mem_range const &o) const
  {
    if (!overlaps(o))
      return Mem_range();

    return Mem_range{start > o.start ? start : o.start,
                      end < o.end ? end : o.end};
  }

  template<typename DBG>
  void
  dump(DBG &out, bool newline = true) const
  {
    static constexpr char const *const unitstr[7] =
    { "Byte", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };

    size_t sz = size();
    unsigned i = 0;
    // Handle size overflow in case region spans the entire address range.
    if (sz == 0 && valid())
      {
        sz = size_t{1} << ((sizeof(size_t) * 8) - 10);
        i = 1;
      }
    for (; i < cxx::array_size(unitstr) && sz > 8 << 10; ++i)
      sz >>= 10;

    if (newline)
      out.printf("%12lx - %12lx (%zu %s)\n", start, end, sz, unitstr[i]);
    else
      out.cprintf("%12lx - %12lx (%zu %s)\n", start, end, sz, unitstr[i]);
  }
};

/**
 * Manages a fixed-size, sorted and overlap free list of regions.
 */
template<typename Region_type, unsigned max_regions>
class Mem_region_list
{
public:
  static constexpr unsigned Max_regions = max_regions;

  /**
   * Add a region.
   *
   * If the region overlaps with existing regions the operation fails.
   * If the region is adjacent to existing regions, it gets merged with them.
   * Otherwise a new region entry is inserted at the position according to the
   * sort order implemented by `Region_type::operator <`.
   *
   * \param  region  Region to add.
   *
   * \retval true   Region added.
   * \retval false  Region is invalid, no free entry is left or region overlaps
   *                with an existing region.
   */
  [[nodiscard]] bool add(Region_type const &region)
  {
    if (!region.valid())
      return false;

    unsigned pos = 0;
    for (; pos < _len && _regions[pos] < region; ++pos)
      ;

    if (pos < _len  && region.overlaps(_regions[pos]))
        // Overlaps are not allowed.
        return false;

    bool prev_adjacent = pos > 0 && region.adjacent(_regions[pos - 1]);
    bool next_adjacent = pos < _len && region.adjacent(_regions[pos]);
    if (prev_adjacent)
      {
        _regions[pos - 1].merge(region);
        if (next_adjacent)
          {
            _regions[pos - 1].merge(_regions[pos]);
            del(pos);
          }
        return true;
      }

    if (next_adjacent)
      {
        _regions[pos].merge(region);
        return true;
      }

    if (_len >= Max_regions)
      return false;

    for (unsigned i = _len; i > pos; --i)
      _regions[i] = _regions[i - 1];
    ++_len;
    _regions[pos] = region;
    return true;
  }

  void del(unsigned pos)
  {
    assert(pos < _len);

    --_len;
    for (unsigned i = pos; i < _len; ++i)
      _regions[i] = _regions[i + 1];
  }

  void clear() { _len = 0; }

  void intersect(Region_type const &region)
  {
    for (int i = static_cast<int>(_len) - 1; i >= 0; --i)
      {
        Region_type intersection = _regions[i].intersect(region);
        if (intersection.valid())
          _regions[i] = intersection;
        else
          del(i);
      }
  }

  bool contains(Region_type const &region) const
  {
    for (auto const &candidate : *this)
      {
        if (candidate.contains(region))
          return true;
      }
    return false;
  }

  template<typename DBG>
  void
  dump(DBG &out) const
  {
    for (auto const &region : *this)
      region.dump(out);
  }

  bool empty() const { return _len == 0; }
  bool full() const { return _len == Max_regions; }
  unsigned len() const { return _len; }
  Region_type const &operator[](unsigned idx) const { return _regions[idx]; }
  Region_type &operator[](unsigned idx) { return _regions[idx]; }

  Region_type *begin() { return _regions; }
  Region_type *end() { return _regions + _len; }
  Region_type const *begin() const { return _regions; }
  Region_type const *end() const { return _regions + _len; }

protected:
  unsigned _len = 0;
  Region_type _regions[Max_regions] = {};
};

} // namespace Moe
