/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dma_space.h"
#include "dataspace.h"
#include <l4/re/error_helper>
#include <l4/cxx/hlist>
#include <l4/sys/task>
#include <l4/cxx/unique_ptr>

// TODO:
//   1. Add the Cache handling for ARM etc.
//   2. check and garbage collect Dma_space_task_mappers when their task capability vanishes
//      (may be also when the last Dma_space associated with the task vanishes)

namespace Moe {
namespace Dma {

class Phys_mapper : public Mapper
{
private:
  typedef Mapping::Map Map;
  Map _map;

public:
  Mapping *map(Dataspace *ds, Q_alloc *alloc, l4_addr_t offset,
               l4_size_t *size, Attributes attrs, Direction dir,
               Dma_addr *dma_addr) override
  {
    L4Re::chksys(ds->dma_map(0, offset, size, attrs, dir, dma_addr));

    cxx::unique_ptr<Dma::Mapping> m(alloc->make_obj<Dma::Mapping>());

    if (!m)
      L4Re::chksys(-L4_ENOMEM);

    m->key = Dma::Region(*dma_addr, *dma_addr + *size -1);
    if (!_map.insert(m.get()).second)
      L4Re::chksys(-L4_EEXIST);

    m->mapper = this;
    m->attrs = attrs;
    m->dir = dir;
    return m.release();
  }

  int unmap(Dma_addr dma_addr, l4_size_t, Attributes, Direction) override
  {
    auto *m = _map.find_node(dma_addr);
    if (!m)
      return -L4_ENOENT;

    // XXX: think about splitting etc.
    delete m;
    return 0;
    //return ds->dma_unmap(0, offset, size, attrs, dir);
  }

  void remove(Dma::Mapping *m) override
  {
    _map.remove(m->key);
    // possibly do the right cache flushing and unpinning
  }
};


class Task_mapper :
  public Mapper,
  public cxx::H_list_item_t<Task_mapper>
{
private:
  static cxx::H_list_t<Task_mapper> _mappers;
  typedef Mapping::Map Map;

  l4_addr_t min = 1 << 20;
  l4_addr_t max = ~0UL;
  Map _map;

  l4_addr_t find_free(l4_addr_t start, l4_addr_t end,
                      unsigned long size, unsigned char align)
  {
    if (size == 0)
      return L4_INVALID_ADDR;

    l4_addr_t a = start;
    if (a < min)
      a = min;

    if (end > max)
      end = max;

    end = l4_trunc_size(end, align);
    if (end <= start)
      return L4_INVALID_ADDR;

    a = l4_round_size(a, align);
    if (a + size - 1 > end)
      return L4_INVALID_ADDR;

    for (;;)
      {
        auto n = _map.find_node(Region(a, a + size - 1));
        if (!n)
          return a;

        a = n->key.end;
        if (a >= end)
          return L4_INVALID_ADDR;

        a = a + 1;
        a = l4_round_size(a, align);
        if (a >= end)
          return L4_INVALID_ADDR;

        if (a + size - 1 > end)
          return L4_INVALID_ADDR;

      }
  }

  L4::Cap<L4::Task> dma_kern_space;

  bool is_equal(L4::Cap<L4::Task> s) const
  {
    L4::Cap<L4::Task> myself(L4_BASE_TASK_CAP);
    return myself->cap_equal(s, dma_kern_space).label();
  }

  void remove(Dma::Mapping *m) override
  {
    _map.remove(m->key);

     l4_addr_t a = m->key.start;
     l4_size_t s = m->key.end - m->key.start + 1;
     unsigned o = L4_PAGESHIFT;
     if (0)
       printf("DMA: unmap %lx-%lx\n", a, a+s-1);
     while (s > 0)
       {
         while ((1UL << o) > s)
           --o;

         while ((1UL << o) <= s && (a & ((1UL << o) - 1)) == 0)
           ++o;

         --o;
         l4_fpage_t fp = l4_fpage(a, o, L4_FPAGE_RWX);

         if (0)
           printf("DMA: unmap   %lx-%lx\n", a, a+(1UL << o)-1);

         dma_kern_space->unmap(fp, L4_FP_ALL_SPACES);
         s -= (1UL << o);
         a += (1UL << o);
       }
  }

public:
  explicit Task_mapper(L4::Cap<L4::Task> s)
  : dma_kern_space(s)
  { _mappers.add(this); }

  ~Task_mapper() noexcept
  {
    if (dma_kern_space)
      object_pool.cap_alloc()->free(dma_kern_space);
  }

  static Task_mapper *find_mapper(L4::Cap<L4::Task> task)
  {
    for (auto m: _mappers)
      if (m->is_equal(task))
        return m;
    return 0;
  }

  Mapping *map(Dataspace *ds, Q_alloc *alloc, l4_addr_t offset,
               l4_size_t *_size, Attributes attrs, Direction dir,
               Dma_space::Dma_addr *dma_addr) override
  {
    if (0)
      printf("DMA %p: map: offs=%lx sz=%zx ...\n", this, offset, *_size);

    // FIXME: do correct page alignment etc.
    offset = l4_trunc_page(offset);

    unsigned long max_sz = ds->round_size();
    if (offset >= max_sz)
      L4Re::chksys(-L4_ERANGE);

    max_sz -= offset;

    if (*_size > max_sz)
      *_size = max_sz;

    l4_size_t size = *_size;
    l4_addr_t a = find_free(min, max, size, L4_SUPERPAGESHIFT); //ds->page_shift());
    if (a == L4_INVALID_ADDR)
      L4Re::chksys(-L4_ENOMEM);

    cxx::unique_ptr<Dma::Mapping> node(alloc->make_obj<Dma::Mapping>());

    if (!node)
      L4Re::chksys(-L4_ENOMEM);

    node->key = Region(a, a + size - 1);
    if (!_map.insert(node.get()).second)
      L4Re::chksys(-L4_ENOMEM);

    node->mapper = this;
    node->attrs = attrs;
    node->dir = dir;

    *dma_addr = a;
    for (;;)
      {
        L4::Ipc::Snd_fpage fpage;
        L4Re::chksys(ds->map(offset, a, Moe::Dataspace::Writable,
                             a, a + size - 1, fpage));

        L4::Cap<L4::Task> myself(L4_BASE_TASK_CAP);

        l4_fpage_t f;
        f.raw = fpage.data();
        L4Re::chksys(dma_kern_space->map(myself, f, a));

        unsigned long s = 1UL << fpage.order();
        if (size <= s)
          break;

        offset += s;
        a += s;
        size -= s;
      }

    return node.release();
  }

  int unmap(Dma_addr dma_addr, l4_size_t, Attributes, Direction) override
  {
    auto *m = _map.find_node(dma_addr);
    if (!m)
      return -L4_ENOENT;

    // XXX: think about node splitting, merging
    delete m;
    //return ds->dma_unmap(0, offset, size, attrs, dir);
    return 0;
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

long
Dma_space::op_map(L4Re::Dma_space::Rights,
                  L4::Ipc::Snd_fpage src_ds, l4_addr_t offset,
                  l4_size_t &size, Attributes attrs, Direction dir,
                  Dma_space::Dma_addr &dma_addr)
{
  if (!_mapper)
    return -L4_EINVAL;

  auto *m =_mapper->map(_get_ds(src_ds), this->qalloc(), offset, &size,
                        attrs, dir, &dma_addr);
  _mappings.add(m);
  return 0;
}

long
Dma_space::op_unmap(L4Re::Dma_space::Rights,
                    Dma_addr dma_addr, l4_size_t size,
                    Attributes attrs, Direction dir)
{
  if (!_mapper)
    return -L4_EINVAL;

  return _mapper->unmap(dma_addr, size, attrs, dir);
}

long
Dma_space::op_associate(L4Re::Dma_space::Rights,
                        L4::Ipc::Snd_fpage dma_task,
                        Space_attribs attr)
{
  _attr = attr;
  if (_mapper)
    {
      delete_all_mappings();
      _mapper = 0;
    }

  if (attr & L4Re::Dma_space::Phys_space)
    {
      _mapper = cxx::Ref_ptr<Dma::Mapper>(qalloc()->make_obj<Dma::Phys_mapper>());
      return 0;
    }
  else
    {
      L4::Cap<L4::Task> rcv_cap(Rcv_cap << L4_CAP_SHIFT);
      if (!dma_task.cap_received())
        return -L4_EINVAL;

      Dma::Mapper *mapper = Dma::Task_mapper::find_mapper(rcv_cap);
      if (!mapper)
        {
          if (0)
            printf("new DMA task assigned, allocate new mapper\n");

          L4::Cap<L4::Task> nc = object_pool.cap_alloc()->alloc<L4::Task>();
          if (!nc.is_valid())
            return -L4_ENOMEM;

          nc.move(rcv_cap);
          mapper = new Dma::Task_mapper(nc);
        }

      _mapper = cxx::Ref_ptr<Dma::Mapper>(mapper);
      return 0;
    }
}

long
Dma_space::op_disassociate(L4Re::Dma_space::Rights)
{
  if (!_mapper)
    return -L4_ENOENT;

  delete_all_mappings();
  _mapper = 0;
  return 0;
};

void
Dma_space::delete_all_mappings()
{
  while (!_mappings.empty())
    delete _mappings.pop_front();
}
}
