/*
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "mem_region_list.h"
#include "page_alloc.h"

#include <l4/cxx/minmax>
#include <l4/sys/consts.h>

#include <cstring>

namespace Moe
{

/**
 * Holds the memory ranges that need page reference counters.
 *
 * The ranges are kept sorted and overlapping or adjacent ranges are merged.
 *
 * The number of ranges is not limited, the ranges are kept in memory from the
 * page allocator. Hence, memory must have been given to the page allocator
 * before the first range is added.
 */
class Ram_ranges
{
public:
  ~Ram_ranges() { free_storage(_r, _capacity); }

  unsigned count() const { return _num; }
  Moe::Mem_range const *begin() const { return _r; }
  Moe::Mem_range const *end() const { return _r + _num; }

  /**
   * Add the memory of the given range to the ranges.
   *
   * \param range  Memory range to add, gets rounded to whole pages.
   *
   * \throws L4::Out_of_memory  No memory left to store another range.
   */
  void add(Moe::Mem_range range)
  {
    if (!range.valid() || range.size() == 0)
      return;

    // Round the range to whole pages. This expands partial pages into full
    // pages, but that is fine since the page reference counters only track use
    // count after allocation, and do not decide whether pages can be allocated.
    range = Moe::Mem_range(l4_trunc_page(range.start),
                           l4_round_page(range.end) - 1);

    // Find the first range that is not completely below the new range.
    unsigned pos = 0;
    while (pos < _num && _r[pos] < range)
      ++pos;

    if (pos < _num && _r[pos].touches(range))
      {
        // Overlapping or adjacent: extend range at `pos`.
        range.merge(_r[pos]);

        // Swallow all following ranges that are overlapped by or adjacent to
        // the enlarged range as well.
        unsigned pos_end = pos + 1;
        for (; pos_end < _num && _r[pos_end].touches(range); ++pos_end)
          range.merge(_r[pos_end]);

        _r[pos] = range;
        remove(pos + 1, pos_end);
        return;
      }

    if (_num == _capacity)
      grow();

    for (unsigned i = _num; i > pos; --i)
      _r[i] = _r[i - 1];

    _r[pos] = range;
    ++_num;
  }

  /**
   * Merge the ranges that are separated by a small hole only.
   *
   * The reference counters for the pages of such a hole cost less than the
   * separate range that avoids them.
   *
   * \param merge_hole_size  Size of hole that is not worth a separate range.
   */
  void merge_small_holes(size_t merge_hole_size)
  {
    if (_num < 2)
      return;

    unsigned dst = 0;
    for (unsigned i = 1; i < _num; ++i)
      {
        // The ranges are sorted and disjoint, hence the hole size cannot
        // underflow.
        if (_r[i].start - _r[dst].end - 1 <= merge_hole_size)
          // Give up the hole.
          _r[dst].end = _r[i].end;
        else
          _r[++dst] = _r[i];
      }

    _num = dst + 1;
  }

private:
  /// Number of ranges the first allocation provides room for.
  static constexpr unsigned Initial_capacity = 8;

  /// Return the storage of `capacity` ranges at `r` to the page allocator.
  static void free_storage(Moe::Mem_range *r, unsigned capacity)
  {
    if (capacity)
      // The memory is given back with _free_internal() and not with _free(),
      // because _free() keeps the memory on kernels without mapdb. That is the
      // right behavior for memory that was handed out to someone else, but this
      // memory here never left Moe.
      Single_page_alloc_base::_free_internal(r, storage_size(capacity));
  }

  /// Size of the storage for `capacity` ranges, allocated in whole pages.
  static unsigned long storage_size(unsigned capacity)
  {
    return l4_round_page(capacity * sizeof(Moe::Mem_range));
  }

  /// Get room for more ranges, throws if there is no memory left.
  void grow()
  {
    unsigned n_capacity = _capacity ? 2 * _capacity : Initial_capacity;
    auto *n = static_cast<Moe::Mem_range *>(
      Single_page_alloc_base::_alloc(storage_size(n_capacity), L4_PAGESIZE));

    if (_num)
      memcpy(n, _r, _num * sizeof(Moe::Mem_range));
    free_storage(_r, _capacity);

    _r = n;
    _capacity = n_capacity;
  }

  /// Drop the ranges [from, to) from the array.
  void remove(unsigned from, unsigned to)
  {
    if (from >= to)
      return;

    for (unsigned i = to; i < _num; ++i)
      _r[i - (to - from)] = _r[i];

    _num -= to - from;
  }

  Moe::Mem_range *_r = nullptr;
  unsigned _num = 0;
  unsigned _capacity = 0;
};

} // namespace Moe
