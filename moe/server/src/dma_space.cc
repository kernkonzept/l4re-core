/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "dma_space.h"
#include "dataspace.h"
#include "name_space.h"

#include <l4/cxx/hlist>
#include <l4/cxx/minmax>
#include <l4/cxx/unique_ptr>
#include <l4/re/error_helper>
#include <l4/sys/cxx/consts>
#include <l4/sys/task>

// TODO:
//   1. check and garbage collect Dma_space_task_mappers when their task
//      capability vanishes (may be also when the last Dma_space associated
//      with the task vanishes)

namespace {

static constexpr bool Debug = false;

L4Re::Dma_space::Dma_addr trunc_dma_addr(L4Re::Dma_space::Dma_addr addr,
                                         unsigned char align = L4_PAGESHIFT)
{
  return L4::trunc_order(addr, align);
}

bool round_dma_size(L4Re::Dma_space::Dma_size *size,
                    unsigned char align = L4_PAGESHIFT)
{
  if (align < L4_PAGESHIFT || align > 30)
    return false;

  auto n = L4::round_order(*size, align);
  if (n < *size)
    return false;

  *size = n;
  return true;
}

/**
 * Round start address up to next alignment.
 */
bool align_start_chk(L4Re::Dma_space::Dma_addr *addr,
                     unsigned char align = L4_PAGESHIFT)
{
  if (align < L4_PAGESHIFT || align > 30)
    return false;

  auto n = L4::round_order(*addr, align);
  if (n < *addr)
    return false;

  *addr = n;
  return true;
}

/**
 * Round down inclusive end address down to next alignment.
 */
bool align_end_chk(L4Re::Dma_space::Dma_addr *addr,
                   unsigned char align = L4_PAGESHIFT)
{
  if (align < L4_PAGESHIFT || align > 30)
    return false;

  auto n = L4::trunc_order(*addr + 1U, align) - 1U;
  if (n > *addr)
    return false;

  *addr = n;
  return true;
}

}

namespace Moe {
namespace Dma {

l4_ret_t Mapper::add_dma_space(Dma_space *dma_space)
{
  _dma_spaces.add(dma_space);
  return 0;
}

void Mapper::remove_dma_space(Dma_space *dma_space)
{ _dma_spaces.remove(dma_space); }


class Phys_mapper final : public Mapper
{
public:
  l4_ret_t map(Dataspace *ds, L4Re::Dataspace::Offset offset,
               L4Re::Dma_space::Dma_size *size, unsigned char,
               L4Re::Dataspace::Flags,
               L4Re::Dma_space::Attributes attrs,
               L4Re::Dma_space::Dma_addr *dma_addr,
               L4Re::Dma_space::Dma_addr) override
  {
    // With physical mappings you cannot choose your address layout.
    if (attrs & L4Re::Dma_space::Reserve)
      return -L4_EPERM;

    if (l4_ret_t err = ds->dma_map(offset, size, dma_addr); err < 0)
      return err;

    return 0;
  }

  void unmap(L4Re::Dma_space::Dma_addr, L4Re::Dma_space::Dma_size) override
  {}

  l4_ret_t check_blocking_area(L4Re::Dma_space::Dma_addr *,
                               L4Re::Dma_space::Dma_addr,
                               L4Re::Dma_space::Dma_size ,
                               bool , unsigned char) override
  { return -L4_EPERM; }

  l4_ret_t set_limits(Dma_space *,
                      L4Re::Dma_space::Dma_addr,
                      L4Re::Dma_space::Dma_addr) override
  { return -L4_EPERM; }
};


/**
 * #Task_mapper instances are associated with L4::Task objects (see
 * #_dma_kern_space). There shall be at most one #Task_mapper instance per
 * L4::Task object (see find_mapper()).
 */
class Task_mapper final :
  public Mapper,
  public cxx::H_list_item_t<Task_mapper>
{
private:
  /// List of all #Task_mapper instances.
  static cxx::H_list_t<Task_mapper> _mappers;

  L4Re::Dma_space::Dma_addr _min = 0;
  L4Re::Dma_space::Dma_addr _max = Last_dma_addr;

  L4::Cap<L4::Task> _dma_kern_space;
  bool _identity_map;

  /**
   * Find any blocking, mapping or reservation in the given region.
   *
   * If there are multiple potential matches, it is unspecified which matching
   * region is returned.
   */
  Mapping const *find_any_region(Region r)
  {
    for (auto i : _dma_spaces)
      if (Mapping const *m = i->find_any_region(r))
        return m;

    return nullptr;
  }

  /// Find first blocking, mapping or reservation in given region.
  Mapping const *find_first_region(Region r)
  {
    Mapping const *ret = nullptr;

    for (auto i : _dma_spaces)
      if (Mapping const *m = i->find_first_region(r))
        if (!ret || ret->key.start > m->key.start)
          ret = m;

    return ret;
  }

  /// Find first blocking or mapping/reservation in given region.
  Mapping const *find_first(Region r, bool find_blocked)
  {
    Mapping const *ret = nullptr;

    for (auto dma_space : _dma_spaces)
      {
        Region search = ret ? Region(r.start, ret->key.start - 1) : r;
        // We need to loop in case we find a region that does not match the
        // `find_blocked` criteria.
        while (auto const *m = dma_space->find_first_region(search))
          {
            if (m->is_blocked() == find_blocked)
              {
                if (m->key.start <= r.start)
                  return m;
                if (!ret || m->key.start < ret->key.start)
                  ret = m;
              }
            else
              {
                // Found a region but it does not match our criteria. Constrain
                // the search region and try again (if possible).
                if (m->key.end >= search.end)
                  break;
                search.start = m->key.end + 1;
              }
          }
      }

    return ret;
  }

  Mapping const *find_any_blocking(Region r)
  { return find_first(r, true); }

  Mapping const *find_first_mapping(Region r)
  { return find_first(r, false); }

  l4_ret_t find_free(L4Re::Dma_space::Dma_size *size, unsigned char align,
                     L4Re::Dma_space::Dma_addr *dma_addr,
                     L4Re::Dma_space::Dma_addr dma_max,
                     bool trunc)
  {
    if (*size == 0)
      return -L4_EINVAL;

    assert(trunc_dma_addr(*size) == *size);

    L4Re::Dma_space::Dma_addr addr = cxx::max(_min, *dma_addr);
    dma_max = cxx::min(_max, dma_max);
    if (!align_start_chk(&addr, align) || !align_end_chk(&dma_max, align))
      return -L4_EINVAL;

    for (;;)
      {
        if (addr > dma_max)
          break;

        if (dma_max - addr < *size - 1)
          {
            if (!trunc)
              break;
            *size = dma_max - addr + 1;
          }

        Region r(addr, addr + *size - 1);
        auto const *n = trunc ? find_first_region(r) : find_any_region(r);
        if (!n)
          {
            *dma_addr = addr;
            return L4_EOK;
          }

        if (trunc && n->key.start > addr
                  && (n->key.start - addr) >= (1UL << align))
          {
            *dma_addr = addr;
            *size = trunc_dma_addr(n->key.start - addr, align);
            return L4_EOK;
          }

        addr = n->key.end;
        if (!align_start_chk(&addr, align))
          break;
      }

    return -L4_EADDRNOTAVAIL;
  }

  l4_ret_t verify_free(L4Re::Dma_space::Dma_size *size,
                       L4Re::Dma_space::Dma_addr dma_addr,
                       L4Re::Dma_space::Dma_addr dma_max,
                       bool ignore_blocked,
                       bool trunc)
  {
    if (*size == 0)
      return -L4_EINVAL;

    assert(trunc_dma_addr(*size) == *size);
    if (dma_addr != trunc_dma_addr(dma_addr))
      return -L4_EINVAL;

    dma_max = cxx::min(_max, dma_max);
    if (!align_end_chk(&dma_max))
      return -L4_EINVAL;

    if (dma_addr > dma_max)
      return -L4_EADDRNOTAVAIL;

    if (dma_max - dma_addr < *size - 1)
      {
        if (!trunc)
          return -L4_EADDRNOTAVAIL;
        *size = dma_max - dma_addr + 1;
      }

    if (dma_addr < _min)
      return -L4_EADDRNOTAVAIL;

    Region r(dma_addr, dma_addr + *size - 1);
    Mapping const *collision = ignore_blocked ? find_first_mapping(r)
                                              : find_first_region(r);
    if (collision)
      {
        if (!trunc)
          return -L4_EADDRNOTAVAIL;

        if (collision->key.start <= dma_addr)
          return -L4_EADDRNOTAVAIL;

        *size = collision->key.start - dma_addr;
      }

    return 0;
  }

  l4_ret_t verify_not_blocked(L4Re::Dma_space::Dma_size size,
                              L4Re::Dma_space::Dma_addr dma_addr,
                              L4Re::Dma_space::Dma_addr dma_max)
  {
    if (size == 0)
      return -L4_EINVAL;

    assert(trunc_dma_addr(size) == size);
    if (dma_addr != trunc_dma_addr(dma_addr))
      return -L4_EINVAL;

    dma_max = cxx::min(_max, dma_max);
    if (!align_end_chk(&dma_max))
      return -L4_EINVAL;

    if (dma_addr > dma_max || dma_max - dma_addr < size - 1)
      return -L4_EADDRNOTAVAIL;

    if (dma_addr < _min)
      return -L4_EADDRNOTAVAIL;

    if (find_any_blocking(Region(dma_addr, dma_addr + size - 1)))
      return -L4_EADDRNOTAVAIL;

    return 0;
  }

  l4_ret_t map_region(Dataspace *ds, L4Re::Dataspace::Offset aligned_offset,
                      L4Re::Dma_space::Dma_addr aligned_dma_addr,
                      L4Re::Dma_space::Dma_size aligned_dma_size,
                      L4Re::Dataspace::Flags flags)
  {
    // Beware the possible truncation below on 32 bit systems!
    l4_addr_t aligned_addr = aligned_dma_addr;
    l4_size_t aligned_size = aligned_dma_size;
    if constexpr (sizeof(l4_addr_t) < sizeof(L4Re::Dma_space::Dma_addr))
      {
        if (   aligned_addr != aligned_dma_addr
            || aligned_size != aligned_dma_size)
          return -L4_EINVAL;

        if (static_cast<l4_addr_t>(-1) - aligned_addr < aligned_size - 1)
          return -L4_EINVAL;
      }

    if constexpr (Debug)
      printf("DMA: map [0x%lx-0x%lx]\n", aligned_addr,
             aligned_addr + aligned_size - 1);

    for (;;)
      {
        L4::Ipc::Snd_fpage fpage;
        l4_ret_t ret = ds->map(aligned_offset, aligned_addr, flags,
                               aligned_addr, aligned_addr + aligned_size - 1,
                               fpage);
        if (ret < 0)
          {
            unmap_region(aligned_dma_addr, aligned_addr - aligned_dma_addr);
            return ret;
          }

        if constexpr (Debug)
          printf("DMA: map   0x%lx-0x%lx -> 0x%lx\n",
                 fpage.base(), fpage.base() + (1UL << fpage.order()) - 1,
                 aligned_addr);

        L4::Cap<L4::Task> myself(L4_BASE_TASK_CAP);

        l4_fpage_t f;
        f.raw = fpage.data();
        ret = l4_error(_dma_kern_space->map(myself, f, aligned_addr));
        if (ret < 0)
          {
            unmap_region(aligned_dma_addr, aligned_addr - aligned_dma_addr);
            return ret;
          }

        unsigned long s = 1UL << fpage.order();
        if (aligned_size <= s)
          break;

        aligned_offset += s;
        aligned_addr += s;
        aligned_size -= s;
      }

    return L4_EOK;
  }

  l4_ret_t unmap_region(L4Re::Dma_space::Dma_addr addr,
                        L4Re::Dma_space::Dma_size size)
  {
    // Beware the possible truncation below on 32 bit systems!
    l4_addr_t a = addr;
    l4_size_t s = size;
    if constexpr (sizeof(l4_addr_t) < sizeof(L4Re::Dma_space::Dma_addr))
      {
        if (a != addr || s != size)
          return -L4_EINVAL;

        if (static_cast<l4_addr_t>(-1) - a < s - 1)
          return -L4_EINVAL;
      }

    unsigned o = L4_PAGESHIFT;

    if constexpr (Debug)
      printf("DMA: unmap [0x%lx-0x%lx]\n", a, a+s-1);

    while (s > 0)
      {
        while ((1UL << o) > s)
          --o;

        while ((1UL << o) <= s && (a & ((1UL << o) - 1)) == 0)
          ++o;

        --o;
        l4_fpage_t fp = l4_fpage(a, o, L4_FPAGE_RWX);

        if constexpr (Debug)
          printf("DMA: unmap   0x%lx-0x%lx\n", a, a+(1UL << o)-1);

        _dma_kern_space->unmap(fp, L4_FP_ALL_SPACES);
        s -= (1UL << o);
        a += (1UL << o);
      }

    return L4_EOK;
  }

  bool is_equal(L4::Cap<L4::Task> s) const
  {
    L4::Cap<L4::Task> myself(L4_BASE_TASK_CAP);
    return myself->cap_equal(s, _dma_kern_space).label();
  }

public:
  explicit Task_mapper(L4::Cap<L4::Task> s, bool identity_map)
  : _dma_kern_space(s), _identity_map(identity_map)
  { _mappers.add(this); }

  ~Task_mapper() noexcept
  {
    if (_dma_kern_space)
      object_pool.cap_alloc()->free(_dma_kern_space);
  }

  static Task_mapper *find_mapper(L4::Cap<L4::Task> task)
  {
    for (auto m: _mappers)
      if (m->is_equal(task))
        return m;
    return 0;
  }

  bool is_identity_map() const
  { return _identity_map; }

  l4_ret_t add_dma_space(Dma_space *dma_space) override
  {
    L4Re::Dma_space::Dma_addr min = cxx::max(_min, dma_space->min_addr());
    L4Re::Dma_space::Dma_addr max = cxx::min(_max, dma_space->max_addr());
    if (l4_ret_t err = set_limits(dma_space, dma_space->min_addr(),
                                  dma_space->max_addr());
        err < 0)
      return err;

    _min = min;
    _max = max;

    return Mapper::add_dma_space(dma_space);
  }

  void remove_dma_space(Dma_space *dma_space) override
  {
    Mapper::remove_dma_space(dma_space);

    _min = 0;
    _max = Last_dma_addr;
    for (auto i : _dma_spaces)
      {
        _min = cxx::max(_min, i->min_addr());
        _max = cxx::min(_max, i->max_addr());
      }
  }

  l4_ret_t map(Dataspace *ds, L4Re::Dataspace::Offset offset,
               L4Re::Dma_space::Dma_size *size, unsigned char align,
               L4Re::Dataspace::Flags flags,
               L4Re::Dma_space::Attributes attrs,
               L4Re::Dma_space::Dma_addr *dma_addr,
               L4Re::Dma_space::Dma_addr  dma_max) override
  {
    if (*size == 0)
      return -L4_EINVAL;

    // Only full pages can be mapped, so work with a rounded offset internally.
    // Attention: L4Re::Dataspace::Offset is always 64 bits!
    L4Re::Dataspace::Offset aligned_offset = L4::trunc_page(offset);

    // For identity mappings, dma_map() takes care of checking the Dataspace
    // limits.
    if (!_identity_map)
      {
        L4Re::Dataspace::Offset max_sz = ds ? ds->size()
                                            : -L4Re::Dataspace::Offset{1};
        if (offset >= max_sz)
          return -L4_ERANGE;

        max_sz -= offset;

        if (*size > max_sz)
          *size = max_sz;
      }
    else if (attrs & L4Re::Dma_space::Reserve)
      // With physical mappings you cannot choose your address layout.
      return -L4_EPERM;

    L4Re::Dma_space::Dma_addr aligned_addr;
    L4Re::Dma_space::Dma_size aligned_size;
    if (_identity_map)
      {
        // Will adjust size and dma_addr to underlying buffer.
        if (l4_ret_t err = ds->dma_map(offset, size, dma_addr); err < 0)
          return err;

        aligned_addr = trunc_dma_addr(*dma_addr);
        aligned_size = *size + (offset - aligned_offset);
        round_dma_size(&aligned_size); // cannot overflow, limited by ds size
      }
    else
      {
        aligned_size = *size + (offset - aligned_offset);
        if (!round_dma_size(&aligned_size))
          return -L4_EINVAL;

        l4_ret_t ret;

        bool trunc = static_cast<bool>(attrs & L4Re::Dma_space::Partial_map);
        if (attrs & L4Re::Dma_space::Search_addr)
          ret = find_free(&aligned_size, align, dma_addr, dma_max, trunc);
        else if (attrs & L4Re::Dma_space::Replace)
          ret = verify_not_blocked(aligned_size, *dma_addr, dma_max);
        else
          ret = verify_free(&aligned_size, *dma_addr, dma_max, false, trunc);

        if (ret < 0)
          return ret;

        // Propagate any truncation done by find_free()/verify_free() back to
        // *size so that the caller (and op_map's add_region()) see the
        // shortened mapping range.
        L4Re::Dma_space::Dma_size avail = aligned_size
                                          - (offset - aligned_offset);
        if (*size > avail)
          *size = avail;

        aligned_addr = *dma_addr;

        // Return the address of the requested offset.
        *dma_addr += (offset - aligned_offset);
      }

    l4_ret_t ret = ds ? map_region(ds, aligned_offset, aligned_addr,
                                   aligned_size, flags)
                      : 0 /* reservation - no mapping */;
    if (ret < 0)
      return ret;

    return 1; // Let Dma_space remember the mapping
  }

  void unmap(L4Re::Dma_space::Dma_addr start,
             L4Re::Dma_space::Dma_size size) override
  {
    Region region(start, start + size - 1);

    // Only unmap pages that are not still referenced by other Dma_space's
    // that use this mapper...
    for (;;)
      {
        Region chunk = region;
        if (auto *m = find_first_mapping(region))
          {
            // Beginning of region covered by other mapping?
            if (m->key.start <= region.start)
              {
                // Yes. Does the mapping cover the whole region?
                if (m->key.end >= region.end)
                  break;

                // Some tail is left...
                region.start = m->key.end + 1;
                continue;
              }
            else
              // No. Some prefix is left. Unmap it...
              chunk.end = m->key.start - 1;
          }

        unmap_region(chunk.start, chunk.end - chunk.start + 1);
        if (chunk.end >= region.end)
          break;

        region.start = chunk.end + 1;
      }
  }

  l4_ret_t check_blocking_area(L4Re::Dma_space::Dma_addr *dma_addr,
                               L4Re::Dma_space::Dma_addr dma_max,
                               L4Re::Dma_space::Dma_size size,
                               bool search, unsigned char align) override
  {
    // Identity map implies that blockings make no sense. Be consistent
    // with Phys_mapper. Also guard against rogue entries in _mappings.
    if (_identity_map)
      return -L4_EPERM;

    if (size == 0)
      return -L4_EINVAL;

    // We allow blocking areas that are already blocked...
    return search ? find_free(&size, align, dma_addr, dma_max, false)
                  : verify_free(&size, *dma_addr, dma_max, true, false);
  }

  l4_ret_t set_limits(Dma_space *dma_space,
                      L4Re::Dma_space::Dma_addr min_addr,
                      L4Re::Dma_space::Dma_addr max_addr) override
  {
    for (auto i : _dma_spaces)
      if (i != dma_space)
        {
          min_addr = cxx::max(min_addr, i->min_addr());
          max_addr = cxx::min(max_addr, i->max_addr());
        }

    if (min_addr > max_addr)
      return -L4_EINVAL;

    if (min_addr > 0 && find_any_region(Region(0, min_addr - 1)))
      return -L4_EADDRNOTAVAIL;

    if (max_addr < Last_dma_addr
        && find_any_region(Region(max_addr + 1, Last_dma_addr)))
      return -L4_EADDRNOTAVAIL;

    _min = min_addr;
    _max = max_addr;
    return L4_EOK;
  }
};

cxx::H_list_t<Task_mapper> Task_mapper::_mappers(true);

} // namespace Dma

l4_ret_t
Dma_space::op_map(L4Re::Dma_space::Rights,
                  L4::Ipc::Snd_fpage src_ds, L4Re::Dataspace::Offset offset,
                  L4Re::Dma_space::Dma_size &size,
                  unsigned char align,
                  L4Re::Dma_space::Attributes attrs,
                  L4Re::Dma_space::Dma_addr &dma_addr,
                  L4Re::Dma_space::Dma_addr dma_max)
{
  if (!_mapper)
    return -L4_EINVAL;

  static constexpr auto Known_flags = L4Re::Dma_space::Search_addr
                                      | L4Re::Dma_space::Partial_map
                                      | L4Re::Dma_space::Reserve
                                      | L4Re::Dma_space::Replace;
  if (attrs & ~Known_flags)
    return -L4_EINVAL;

  if (   (attrs & L4Re::Dma_space::Replace)
      && (attrs & (L4Re::Dma_space::Search_addr | L4Re::Dma_space::Partial_map)))
    return -L4_EINVAL;

  L4Re::Dataspace::Flags flags(0);
  Dataspace *ds = nullptr;

  if (!(attrs & L4Re::Dma_space::Reserve))
    {
      if (!src_ds.id_received())
        return -L4_EINVAL;

      ds = dynamic_cast<Dataspace *>(object_pool.find(src_ds.data()));
      if (!ds)
        return -L4_EINVAL;

      flags = (src_ds.data() & L4_CAP_FPAGE_W) ? L4Re::Dataspace::F::RW
                                               : L4Re::Dataspace::F::R;
      flags &= ds->flags();
    }
  else if (offset != 0)
    return -L4_EINVAL;

  if constexpr (Debug)
    printf("DMA %p: map: ds=%p offs=0x%llx sz=0x%llx align=%d attrs=0x%x dma_addr=0x%llx dma_max=0x%llx\n",
           this, ds, offset, size, align, attrs.raw, dma_addr, dma_max);

  l4_ret_t res = _mapper->map(ds, offset, &size, align, flags, attrs, &dma_addr,
                              dma_max);
  if (res < 0)
    return res;

  auto add_type = ds ? Add::Mapping : Add::Reservation;

  // Only track regions if mapper needs that.
  if (res > 0)
    {
      // map() returns unaligned addresses if `offset` was not page aligned.
      // All mapping and tracking is done on page granularity so we have to
      // expand the region accordingly.
      L4Re::Dma_space::Dma_addr start = trunc_dma_addr(dma_addr);
      L4Re::Dma_space::Dma_addr end = L4::round_page(dma_addr + size) - 1;
      if ((res = add_region(start, end, add_type)) < 0)
        {
          _mapper->unmap(dma_addr, size);
          return res;
        }
    }

  return 0;
}

l4_ret_t
Dma_space::op_unmap(L4Re::Dma_space::Rights,
                    L4Re::Dma_space::Dma_addr addr,
                    L4Re::Dma_space::Dma_size size,
                    L4Re::Dma_space::Unmap_flags flags)
{
  if (!_mapper)
    return -L4_EINVAL;

  static constexpr auto Known_flags = L4Re::Dma_space::Cancel_reservation;
  if (flags & ~Known_flags)
    return -L4_EINVAL;

  if (size == 0 || Dma::Last_dma_addr - addr < size - 1)
    return -L4_EINVAL;

  // Expand to page granularity
  size += addr - trunc_dma_addr(addr);
  if (!round_dma_size(&size))
    return -L4_EINVAL;
  addr = trunc_dma_addr(addr);

  L4Re::Dma_space::Dma_addr end = addr + size - 1;

  // First pass: verify that no blocked region intersects the range and
  // determine the leftmost and rightmost intersecting regions so that any
  // required split nodes can be pre-allocated.
  Dma::Mapping *first_in_range = nullptr;
  Dma::Mapping *last_in_range = nullptr;
  {
    Dma::Region scan(addr, end);
    while (auto *m = find_first_region(scan))
      {
        if (m->is_blocked())
          return -L4_EPERM;
        if (!first_in_range)
          first_in_range = m;
        last_in_range = m;
        if (m->key.end >= end)
          break;
        scan.start = m->key.end + 1;
      }
  }

  // Split partially intersecting nodes at start or end. Splitting is not
  // visible to the caller so we can do it one after the other, even if the
  // second split fails.
  if (first_in_range && first_in_range->key.start < addr)
    {
      cxx::unique_ptr<Dma::Mapping> fsplit(qalloc()->make_obj<Dma::Mapping>());
      if (!fsplit)
        return -L4_ENOMEM;

      fsplit->key = Dma::Region(first_in_range->key.start, addr - 1);
      fsplit->blocked = first_in_range->blocked;
      fsplit->mapcnt = first_in_range->mapcnt;
      fsplit->rsvcnt = first_in_range->rsvcnt;

      first_in_range->key.start = addr;
      assert(_mappings.insert(fsplit.get()).second);
      fsplit.release();
    }

  if (last_in_range && last_in_range->key.end > end)
    {
      cxx::unique_ptr<Dma::Mapping> lsplit(qalloc()->make_obj<Dma::Mapping>());
      if (!lsplit)
        return -L4_ENOMEM;

      lsplit->key = Dma::Region(addr + size, last_in_range->key.end);
      lsplit->blocked = last_in_range->blocked;
      lsplit->mapcnt = last_in_range->mapcnt;
      lsplit->rsvcnt = last_in_range->rsvcnt;

      last_in_range->key.end = addr + size - 1;
      assert(_mappings.insert(lsplit.get()).second);
      lsplit.release();
    }

  // Second pass: decrement counters and unmap. All checks and allocations
  // have succeeded, so this pass cannot fail.
  while (size)
    {
      auto *m = find_first_region(Dma::Region(addr, addr + size - 1));
      if (!m)
        break;

      assert(m->key.start >= addr && m->key.end <= addr + size - 1);

      size -= m->key.end - addr + 1;
      addr = m->key.end + 1;

      bool unmap = false;
      if (flags & L4Re::Dma_space::Cancel_reservation)
        m->del_reservation();
      else if (m->del_mapping())
        unmap = true;

      if (!m->is_referenced())
        {
          Dma::Region chunk = m->key;

          // Remove from region map before calling Mapper::unmap(). The
          // Task_mapper unmap() will look into all associated Dma_space's to
          // unmap only truely free regions.
          _mappings.remove(m->key);
          delete m;

          if (unmap)
            _mapper->unmap(chunk);
        }
    }

  return 0;
}

l4_ret_t
Dma_space::associate(cxx::Ref_ptr<Dma::Mapper> const &mapper)
{
  // Reject instead of replacing the mapper. We want to keep all pre-associated
  // blockings (op_block_area)!
  if (_mapper)
    return -L4_EBUSY;

  // Verify that a-priori blockings cause no collisions.
  for (auto &i : _mappings)
    if (l4_ret_t err = mapper->check_blocking_area(&i.key.start,
                                                   Dma::Last_dma_addr,
                                                   i.key.end - i.key.start + 1,
                                                   false, 0);
        err < 0)
      return err;

  _mapper = mapper;
  return _mapper->add_dma_space(this);
}

l4_ret_t
Dma_space::disassociate()
{
  _mappings.remove_all([this](Dma::Mapping *m)
    {
      // Only entries that actually own a kernel DMA mapping need a
      // Mapper::unmap() call; pure Reserve claims and blocked areas do not.
      if (!m->is_blocked() && m->has_mappings())
        {
          assert(_mapper);
          _mapper->unmap(m->key);
        }
      delete m;
    });

  if (_mapper)
    {
      _mapper->remove_dma_space(this);
      _mapper = nullptr;
    }

  return 0;
};

l4_ret_t
Dma_space::block_area(L4Re::Dma_space::Dma_addr *start,
                      L4Re::Dma_space::Dma_addr dma_max,
                      L4Re::Dma_space::Dma_size size,
                      bool search, unsigned char align)
{
  if constexpr (Debug)
    printf("DMA %p: block_area: start=0x%llx sz=%llx\n", this, *start, size);

  if (size == 0)
    return -L4_EINVAL;

  if (trunc_dma_addr(size) != size)
    return -L4_EINVAL;

  // It is allowed to register blocked areas before a mapper is associated.
  // This is required to have no time window between associate() and map()
  // where potentially blocked regions could be used.
  if (_mapper)
    {
      l4_ret_t err = _mapper->check_blocking_area(start, dma_max, size, search,
                                                  align);
      if (err < 0)
        return err;
    }
  else if (search)
    // Without mapper, we cannot search for suitable addresses.
    return -L4_EINVAL;
  else
    {
      // Verify all input parameters. Because there might be no _mapper yet, we
      // must not store incorrect values.
      if (trunc_dma_addr(*start) != *start)
        return -L4_EINVAL;

      dma_max = cxx::min(_max, dma_max);
      if (!align_end_chk(&dma_max))
        return -L4_EINVAL;

      if (*start > dma_max || *start < _min)
        return -L4_EADDRNOTAVAIL;

      if (dma_max - *start < size - 1)
        return -L4_EADDRNOTAVAIL;
    }

  if (l4_ret_t err = add_region(*start, *start + size - 1, Add::Block);
      err < 0)
    return err;

  return L4_EOK;
}

l4_ret_t
Dma_space::set_limits(L4Re::Dma_space::Dma_addr min_addr,
                      L4Re::Dma_space::Dma_addr max_addr)
{
  if (trunc_dma_addr(min_addr) != min_addr)
    return -L4_EINVAL;
  if (trunc_dma_addr(max_addr + 1) != max_addr + 1)
    return -L4_EINVAL;

  if (min_addr > max_addr)
    return -L4_EINVAL;

  if constexpr (Debug)
    printf("DMA %p: set_limits: min=%llx max=%llx\n", this, min_addr, max_addr);

  if (_mapper)
    {
      l4_ret_t err = _mapper->set_limits(this, min_addr, max_addr);
      if (err < 0)
        return err;
    }

  _min = min_addr;
  _max = max_addr;

  return L4_EOK;
}

l4_ret_t
Dma_space::add_region(L4Re::Dma_space::Dma_addr start,
                      L4Re::Dma_space::Dma_addr end,
                      Add type)
{
  // Split partially overlapping regions at start and end. The loop below
  // relies on the property that existing regions are fully covered.
  if (auto *front = _mappings.find_node(start))
    if (front->key.start < start)
      {
        cxx::unique_ptr<Dma::Mapping> node(qalloc()->make_obj<Dma::Mapping>());
        if (!node)
          return -L4_ENOMEM;

        node->key = Dma::Region(start, front->key.end);
        node->blocked = front->blocked;
        node->mapcnt = front->mapcnt;

        front->key.end = start - 1;
        assert(_mappings.insert(node.get()).second);

        node.release();
      }

  if (auto *tail = _mappings.find_node(end))
    if (tail->key.end > end)
      {
        cxx::unique_ptr<Dma::Mapping> node(qalloc()->make_obj<Dma::Mapping>());
        if (!node)
          return -L4_ENOMEM;

        node->key = Dma::Region(tail->key.start, end);
        node->blocked = tail->blocked;
        node->mapcnt = tail->mapcnt;

        tail->key.start = end + 1;
        assert(_mappings.insert(node.get()).second);

        node.release();
      }

  // Helper to undo additions in case things go wrong...
  auto undo = [this, start, type](L4Re::Dma_space::Dma_addr until)
    {
      if (until <= start)
        return;

      Dma::Region range(start, until - 1);
      while (auto *m = find_first_region(range))
        {
          auto end = m->key.end;
          switch (type)
            {
            case Add::Mapping: m->del_mapping(); break;
            case Add::Reservation: m->del_reservation(); break;
            case Add::Block: m->del_block(); break;
            }

          if (!m->is_referenced())
            {
              _mappings.remove(m->key);
              delete m;
            }

          if (end >= range.end)
            break;

          range.start = end + 1;
        }
    };

  // Iterate over range, incrementing reference count of existing regions or
  // filling the gaps with new regions.
  L4Re::Dma_space::Dma_addr addr = start;
  L4Re::Dma_space::Dma_size remaining = end - start + 1;
  while (remaining)
    {
      Dma::Region new_reg(addr, addr + remaining - 1);
      auto *existing = find_first_region(new_reg);
      if (existing)
        {
          if (existing->key.start == addr)
            {
              bool ok = false;
              switch (type)
                {
                case Add::Mapping:
                  assert(!existing->is_blocked());
                  ok = existing->add_mapping();
                  break;
                case Add::Reservation:
                  assert(!existing->is_blocked());
                  ok = existing->add_reservation();
                  break;
                case Add::Block:
                  assert(existing->is_blocked());
                  ok = existing->add_block();
                  break;
                }

              if (!ok)
                {
                  undo(addr);
                  return -L4_ERANGE;
                }

              remaining -= existing->key.end - addr + 1;
              addr = existing->key.end + 1;
              continue;
            }
          else
            new_reg.end = existing->key.start - 1;
        }

      cxx::unique_ptr<Dma::Mapping> node(qalloc()->make_obj<Dma::Mapping>());
      if (!node)
        {
          undo(addr);
          return -L4_ENOMEM;
        }

      node->key = new_reg;
      switch (type)
        {
        case Add::Mapping: node->mapcnt = 1; break;
        case Add::Reservation: node->rsvcnt = 1; break;
        case Add::Block: node->blocked = 1; break;
        }
      assert(_mappings.insert(node.get()).second);
      node.release();

      remaining -= new_reg.end - new_reg.start + 1;
      addr = new_reg.end + 1;
    }

  return 0;
}


l4_ret_t
Dma_space_mgr::check_dma_space(L4::Ipc::Snd_fpage const &dma_space,
                               Moe::Dma_space **res)
{
  if (!dma_space.id_received())
    return -L4_ENOENT;

  if (!(dma_space.data() & L4_CAP_FPAGE_W))
    return -L4_EPERM;

  auto *moe_dma_space = dynamic_cast<Moe::Dma_space*>(object_pool.find(dma_space.data()));

  if (!moe_dma_space)
    return -L4_ENOENT;

  *res = moe_dma_space;
  return L4_EOK;
}

l4_ret_t
Dma_space_mgr::op_associate(L4Re::Dma_space_mgr::Rights,
                            L4::Ipc::Snd_fpage dma_space_cap,
                            L4::Ipc::Snd_fpage dma_task,
                            L4Re::Dma_space_mgr::Space_attribs attr)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  L4::Cap<L4::Task> rcv_cap(Rcv_cap2 << L4_CAP_SHIFT);
  if (!dma_task.cap_received())
    return -L4_EINVAL;

  static constexpr auto Known_flags = L4Re::Dma_space_mgr::Identity_map;
  if (attr & ~Known_flags)
    return -L4_EINVAL;

  bool ident_map = static_cast<bool>(attr & L4Re::Dma_space_mgr::Identity_map);

  // No a-priori blockings allowed with identity mappings. They may collide
  // with RAM.
  if (ident_map)
    {
      if (!dma_space->empty())
        return -L4_EINVAL;
      if (!dma_space->unconstrained())
        return -L4_EINVAL;
    }

  Dma::Task_mapper *mapper = Dma::Task_mapper::find_mapper(rcv_cap);
  if (mapper)
    {
      if (mapper->is_identity_map() != ident_map)
        return -L4_EINVAL;
    }
  else
    {
      if constexpr (Debug)
        printf("new DMA task assigned, allocate new mapper\n");

      L4::Cap<L4::Task> nc = object_pool.cap_alloc()->alloc<L4::Task>();
      if (!nc.is_valid())
        return -L4_ENOMEM;

      nc.move(rcv_cap);
      mapper = new Dma::Task_mapper(nc, ident_map);
    }

  return dma_space->associate(cxx::Ref_ptr<Dma::Mapper>(mapper));
}

l4_ret_t
Dma_space_mgr::op_associate_phys(L4Re::Dma_space_mgr::Rights,
                                 L4::Ipc::Snd_fpage dma_space_cap,
                                 L4Re::Dma_space_mgr::Space_attribs attr)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  if (attr)
    return -L4_EINVAL;

  // No a-priori blockings allowed with physical mappings.
  if (!dma_space->empty() || !dma_space->unconstrained())
    return -L4_EINVAL;

  return dma_space->associate(cxx::Ref_ptr<Dma::Mapper>(
    dma_space->qalloc()->make_obj<Dma::Phys_mapper>()));
}

l4_ret_t
Dma_space_mgr::op_disassociate(L4Re::Dma_space_mgr::Rights,
                               L4::Ipc::Snd_fpage dma_space_cap)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  return dma_space->disassociate();
};

l4_ret_t
Dma_space_mgr::op_block_area(L4Re::Dma_space_mgr::Rights,
                             L4::Ipc::Snd_fpage                 dma_space_cap,
                             L4Re::Dma_space::Dma_addr          &start,
                             L4Re::Dma_space::Dma_size          size,
                             L4Re::Dma_space::Dma_addr          max,
                             L4Re::Dma_space_mgr::Block_flags   flags,
                             unsigned char                      align)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  static constexpr auto Known_flags = L4Re::Dma_space_mgr::Search_addr;
  if (flags & ~Known_flags)
    return -L4_EINVAL;

  bool search = static_cast<bool>(flags & L4Re::Dma_space_mgr::Search_addr);
  return dma_space->block_area(&start, max, size, search, align);
}

l4_ret_t
Dma_space_mgr::op_set_limits(L4Re::Dma_space_mgr::Rights,
                             L4::Ipc::Snd_fpage        dma_space_cap,
                             L4Re::Dma_space::Dma_addr min_addr,
                             L4Re::Dma_space::Dma_addr max_addr)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  return dma_space->set_limits(min_addr, max_addr);
}

Dma_space_mgr::Dma_space_mgr(Moe::Name_space *ns, char const *name)
{
  object_pool.cap_alloc()->alloc(this, name);
  ns->register_obj(name, Entry::F_rw, this);
}

}
