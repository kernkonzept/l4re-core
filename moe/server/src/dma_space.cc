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
               L4Re::Dma_space::Dma_size *size,
               L4Re::Dma_space::Dma_addr *dma_addr) override
  {
    if (l4_ret_t err = ds->dma_map(offset, size, dma_addr); err < 0)
      return err;

    return 0;
  }

  void unmap(L4Re::Dma_space::Dma_addr, L4Re::Dma_space::Dma_size) override
  {}
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

  L4Re::Dma_space::Dma_addr _min = 1 << 20;
  L4Re::Dma_space::Dma_addr _max = ~0UL;

  L4::Cap<L4::Task> _dma_kern_space;

  /**
   * Find any mapping in the given region.
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

  /// Find first mapping in given region.
  Mapping const *find_first_region(Region r)
  {
    Mapping const *ret = nullptr;

    for (auto i : _dma_spaces)
      if (Mapping const *m = i->find_first_region(r))
        if (!ret || ret->key.start > m->key.start)
          ret = m;

    return ret;
  }

  l4_ret_t find_free(L4Re::Dma_space::Dma_size *size,
                     L4Re::Dma_space::Dma_addr *dma_addr)
  {
    assert(trunc_dma_addr(*size) == *size);

    L4Re::Dma_space::Dma_addr addr = _min;

    for (;;)
      {
        if (addr > _max || _max - addr < *size - 1)
          break;

        auto const *n = find_any_region(Region(addr, addr + *size - 1));
        if (!n)
          {
            *dma_addr = addr;
            return L4_EOK;
          }

        addr = n->key.end;
        if (!align_start_chk(&addr))
          break;
      }

    return -L4_EADDRNOTAVAIL;
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
  explicit Task_mapper(L4::Cap<L4::Task> s)
  : _dma_kern_space(s)
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

  l4_ret_t map(Dataspace *ds, L4Re::Dataspace::Offset offset,
               L4Re::Dma_space::Dma_size *size,
               L4Re::Dma_space::Dma_addr *dma_addr) override
  {
    if (*size == 0)
      return -L4_EINVAL;

    // Only full pages can be mapped, so work with a rounded offset internally.
    // Attention: L4Re::Dataspace::Offset is always 64 bits!
    L4Re::Dataspace::Offset aligned_offset = L4::trunc_page(offset);

    unsigned long max_sz = ds->round_size();
    if (offset >= max_sz)
      return -L4_ERANGE;

    max_sz -= offset;

    if (*size > max_sz)
      *size = max_sz;

    L4Re::Dma_space::Dma_size aligned_size = *size + (offset - aligned_offset);
    round_dma_size(&aligned_size); // cannot overflow because size <= max_sz

    l4_ret_t ret = find_free(&aligned_size, dma_addr);

    if (ret < 0)
      return ret;

    L4Re::Dma_space::Dma_addr aligned_addr = *dma_addr;

    // Return the address of the requested offset.
    *dma_addr += (offset - aligned_offset);

    ret = map_region(ds, aligned_offset, aligned_addr,
                     aligned_size, L4Re::Dataspace::F::RW);
    if (ret < 0)
      return ret;

    return L4_EOK;
  }

  void unmap(L4Re::Dma_space::Dma_addr start,
             L4Re::Dma_space::Dma_size size) override
  {
    unmap_region(start, size);
  }
};

cxx::H_list_t<Task_mapper> Task_mapper::_mappers(true);

} // namespace Dma

static Dataspace *_get_ds(L4::Ipc::Snd_fpage src_cap)
{
  if (!src_cap.id_received())
    L4Re::chksys(-L4_EINVAL);

  if (!(src_cap.data() & L4_CAP_FPAGE_W))
    L4Re::chksys(-L4_EPERM);

  Dataspace *src
    = dynamic_cast<Dataspace*>(object_pool.find(src_cap.data()));

  if (!src)
    L4Re::chksys(-L4_EINVAL);

  return src;
}

l4_ret_t
Dma_space::op_map(L4Re::Dma_space::Rights,
                  L4::Ipc::Snd_fpage src_ds, L4Re::Dataspace::Offset offset,
                  L4Re::Dma_space::Dma_size &size,
                  L4Re::Dma_space::Attributes,
                  L4Re::Dma_space::Dma_addr &dma_addr)
{
  if (!_mapper)
    return -L4_EINVAL;

  l4_ret_t res = _mapper->map(_get_ds(src_ds), offset, &size, &dma_addr);
  if (res < 0)
    return res;

  // map() returns unaligned addresses if `offset` was not page aligned. All
  // mapping and tracking is done on page granularity so we have to expand the
  // region accordingly.
  L4Re::Dma_space::Dma_addr start = trunc_dma_addr(dma_addr);
  L4Re::Dma_space::Dma_addr end = L4::round_page(dma_addr + size) - 1;
  if ((res = add_region(start, end)) < 0)
    {
      _mapper->unmap(dma_addr, size);
      return res;
    }

  return 0;
}

l4_ret_t
Dma_space::op_unmap(L4Re::Dma_space::Rights,
                    L4Re::Dma_space::Dma_addr addr,
                    L4Re::Dma_space::Dma_size size)
{
  if (!_mapper)
    return -L4_EINVAL;

  if (size == 0 || L4Re::Dma_space::Dma_addr(-1) - addr < size - 1)
    return -L4_EINVAL;

  // Expand to page granularity
  size += addr - trunc_dma_addr(addr);
  if (!round_dma_size(&size))
    return -L4_EINVAL;
  addr = trunc_dma_addr(addr);

  while (size)
    {
      auto *m = find_first_region(Dma::Region(addr, addr + size - 1));
      if (!m)
        break;

      _mapper->unmap(m->key);

      if (m->key.end - addr + 1 >= size)
        size = 0;
      else
        size -= m->key.end - addr + 1;
      addr = m->key.end + 1;

      _mappings.remove(m->key);
      delete m;
    }

  return 0;
}

l4_ret_t
Dma_space::associate(cxx::Ref_ptr<Dma::Mapper> const &mapper)
{
  if (_mapper)
    disassociate();

  _mapper = mapper;
  return _mapper->add_dma_space(this);
}

l4_ret_t
Dma_space::disassociate()
{
  _mappings.remove_all([this](Dma::Mapping *m)
    {
      assert(_mapper);
      _mapper->unmap(m->key);
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
Dma_space::add_region(L4Re::Dma_space::Dma_addr start,
                      L4Re::Dma_space::Dma_addr end)
{
  cxx::unique_ptr<Dma::Mapping> node(qalloc()->make_obj<Dma::Mapping>());
  if (!node)
    return -L4_ENOMEM;

  node->key = Dma::Region(start, end);
  if (!_mappings.insert(node.get()).second)
    return -L4_EADDRNOTAVAIL;

  node.release();
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
                            L4Re::Dma_space_mgr::Space_attribs)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  L4::Cap<L4::Task> rcv_cap(Rcv_cap2 << L4_CAP_SHIFT);
  if (!dma_task.cap_received())
    return -L4_EINVAL;

  Dma::Task_mapper *mapper = Dma::Task_mapper::find_mapper(rcv_cap);
  if (!mapper)
    {
      if constexpr (Debug)
        printf("new DMA task assigned, allocate new mapper\n");

      L4::Cap<L4::Task> nc = object_pool.cap_alloc()->alloc<L4::Task>();
      if (!nc.is_valid())
        return -L4_ENOMEM;

      nc.move(rcv_cap);
      mapper = new Dma::Task_mapper(nc);
    }

  return dma_space->associate(cxx::Ref_ptr<Dma::Mapper>(mapper));
}

l4_ret_t
Dma_space_mgr::op_associate_phys(L4Re::Dma_space_mgr::Rights,
                                 L4::Ipc::Snd_fpage dma_space_cap,
                                 L4Re::Dma_space_mgr::Space_attribs)
{
  Dma_space *dma_space;
  l4_ret_t r = check_dma_space(dma_space_cap, &dma_space);
  if (r != L4_EOK)
    return r;

  return dma_space->associate(cxx::Ref_ptr<Dma::Mapper>(dma_space->qalloc()->make_obj<Dma::Phys_mapper>()));
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

Dma_space_mgr::Dma_space_mgr(Moe::Name_space *ns, char const *name)
{
  object_pool.cap_alloc()->alloc(this, name);
  ns->register_obj(name, Entry::F_rw, this);
}

}
